#include "psyqlib.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <array>

#define READ_FIELD(field) m_instream.read(reinterpret_cast<char*>(&field), sizeof(field))

PSYQLib::PSYQLib(const std::string &infile): m_currentlink(NULL), m_currentsection(NULL)
{
    m_statehandler[PSYQState::SectionEOF] = std::bind(&PSYQLib::executeStateSectionEof, this);
    m_statehandler[PSYQState::SectionCode] = std::bind(&PSYQLib::executeStateSectionCode, this);
    m_statehandler[PSYQState::SectionSwitch] = std::bind(&PSYQLib::executeStateSectionSwitch, this);
    m_statehandler[PSYQState::SectionBss] = std::bind(&PSYQLib::executeStateSectionBss, this);
    m_statehandler[PSYQState::SectionPatch] = std::bind(&PSYQLib::executeStateSectionPatch, this);
    m_statehandler[PSYQState::SymbolRef] = std::bind(&PSYQLib::executeStateSymbolRef, this);
    m_statehandler[PSYQState::SymbolDef] = std::bind(&PSYQLib::executeStateSymbolDef, this);
    m_statehandler[PSYQState::SectionSymbol] = std::bind(&PSYQLib::executeStateSectionSymbol, this);
    m_statehandler[PSYQState::Processor] = std::bind(&PSYQLib::executeStateProcessor, this);
    m_statehandler[PSYQState::SymbolBss] = std::bind(&PSYQLib::executeStateSymbolBss, this);

    m_patchhandler[PSYQPatchState::PatchToReference] = std::bind(&PSYQLib::executeStatePatchToReference, this);
    m_patchhandler[PSYQPatchState::PatchToSection] = std::bind(&PSYQLib::executeStatePatchToSection, this);
    m_patchhandler[PSYQPatchState::PatchToValue] = std::bind(&PSYQLib::executeStatePatchToValue, this);
    m_patchhandler[PSYQPatchState::PatchToDiff] = std::bind(&PSYQLib::executeStatePatchToValue, this);

    m_instream.open(infile, std::ios::in | std::ios::binary);
}

const std::list<PSYQModule> &PSYQLib::modules() const { return m_modules; }

bool PSYQLib::load()
{
    if(!m_instream.is_open())
        return false;

    m_instream.read(reinterpret_cast<char*>(&m_header), sizeof(PSYQFile));

    if(std::strncmp(m_header.signature, PSYQ_LIB_SIGNATURE, PSYQ_LIB_SIGNATURE_SIZE) || (m_header.version != 1))
        return false;

    return this->readAllModules();
}

bool PSYQLib::executeStateSectionEof()
{
    m_currentlink = NULL;
    m_currentsection = NULL;
    return true;
}

bool PSYQLib::executeStateSectionCode()
{
    u16 size = 0;
    READ_FIELD(size);

    m_currentsection->code.resize(size);
    m_instream.read(reinterpret_cast<char*>(m_currentsection->code.data()), size);
    return true;
}

bool PSYQLib::executeStatePatchToSection()
{
    PSYQPatch& patch = m_currentlink->patches.back();
    READ_FIELD(patch.sectionnumber);
    return true;
}

bool PSYQLib::executeStateSectionSwitch()
{
    u16 sectionnumber = 0;
    READ_FIELD(sectionnumber);

    auto it = m_currentlink->sections.find(sectionnumber);

    if(it == m_currentlink->sections.end())
        return false;

    m_currentsection = &it->second;
    return true;
}

bool PSYQLib::executeStateSectionBss() { READ_FIELD(m_currentsection->sizebss); return true; }

bool PSYQLib::executeStateSectionPatch()
{
    PSYQPatch patch;
    patch.patchsection = m_currentsection->symbolnumber; // Remember the current section

    READ_FIELD(patch.type);
    READ_FIELD(patch.offset);

    m_currentlink->patches.push_back(patch);
    auto it = m_patchhandler.end();

    while(m_instream)
    {
        PSYQPatchState patchstate;
        m_instream.read(reinterpret_cast<char*>(&patchstate), sizeof(PSYQState));
        it = m_patchhandler.find(patchstate);

        if(it == m_patchhandler.end())
            break;

        it->second();
    }

    if(m_instream)
        m_instream.seekg(-1, std::ios::cur);

    return true;
}

bool PSYQLib::executeStateSymbolRef()
{
    PSYQReference ref;

    READ_FIELD(ref.symbolnumber);
    ref.name = this->readString();

    m_currentlink->references[ref.symbolnumber] = ref;
    return true;
}

bool PSYQLib::executeStateSymbolDef()
{
    PSYQDefinition def;

    READ_FIELD(def.symbolnumber);
    READ_FIELD(def.sectionnumber);
    READ_FIELD(def.offset);
    def.name = this->readString();

    m_currentlink->definitions[def.symbolnumber] = def;
    return true;
}

bool PSYQLib::executeStateSectionSymbol()
{
    PSYQSection section;

    READ_FIELD(section.symbolnumber);
    READ_FIELD(section.group);
    READ_FIELD(section.alignment);
    section.name = this->readString();

    m_currentlink->sections[section.symbolnumber] = section;
    return true;
}

bool PSYQLib::executeStatePatchToValue()
{
    PSYQPatch& patch = m_currentlink->patches.back();
    READ_FIELD(patch.patchvalue.unk1);
    READ_FIELD(patch.patchvalue.value);
    return true;
}

bool PSYQLib::executeStateProcessor() { READ_FIELD(m_currentlink->processorType); return true; }

bool PSYQLib::executeStateSymbolBss()
{
    PSYQBss bss;

    READ_FIELD(bss.symbolnumber);
    READ_FIELD(bss.sectionnumber);
    READ_FIELD(bss.size);
    bss.name = this->readString();

    m_currentlink->bss[bss.symbolnumber] = bss;
    return true;
}

bool PSYQLib::executeStatePatchToReference()
{
    PSYQPatch& patch = m_currentlink->patches.back();
    READ_FIELD(patch.symbolnumber);
    return true;
}

std::list<std::string> PSYQLib::readStringTable()
{
    std::list<std::string> strings;

    while(m_instream)
    {
        std::string s = this->readString();

        if(s.empty())
            break;

        strings.push_back(s);
    }

    return strings;
}

time_t PSYQLib::timestamp(u32 wdatetime) const
{
    struct tm t{0};

    u16 wdate = (wdatetime & 0xFFFF0000) >> 16;
    u16 wtime = static_cast<u16>(wdatetime & 0x0000FFFF);

    t.tm_year = (((wdate & 0xFE00) >> 9) + 1980) - 1900;
    t.tm_mon = ((wdate & 0x01E0) >> 5) - 1;
    t.tm_mday = wdate & 0x001F;

    t.tm_hour = (wtime & 0xF800) >> 11;
    t.tm_min = (wtime & 0x07E0) >> 5;
    t.tm_sec =  (wtime & 0x001F * 2);

    return mktime(&t);
}

std::string PSYQLib::readString()
{
    u8 length = 0;
    std::vector<char> buffer;

    if(m_instream.read(reinterpret_cast<char*>(&length), sizeof(u8)) || (length > 0))
    {
        buffer.resize(length + 1, 0);

        if(!m_instream.read(buffer.data(), length))
            return std::string();
    }

    return buffer.data();
}

PSYQModule PSYQLib::readModule()
{
    PSYQModule module;

    std::array<char, PSYQ_MODULE_NAME_LENGTH> modulename;
    m_instream.read(modulename.data(), PSYQ_MODULE_NAME_LENGTH);
    module.name = std::string(modulename.begin(), modulename.end());

    // Trim name
    module.name.erase(std::find_if(module.name.rbegin(), module.name.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), module.name.end());

    READ_FIELD(module.win_datetime);
    READ_FIELD(module.offsetlink);
    READ_FIELD(module.offsetnext);

    module.names = this->readStringTable();
    return module;
}

PSYQState PSYQLib::readState()
{
    PSYQState state;
    m_instream.read(reinterpret_cast<char*>(&state), sizeof(PSYQState));
    return state;
}

bool PSYQLib::readLink()
{
    m_instream.read(m_currentlink->signature, 3);
    READ_FIELD(m_currentlink->version);

    while(m_instream)
    {
        PSYQState state = this->readState();
        auto it = m_statehandler.find(state);

        if(it == m_statehandler.end())
        {
            std::cout << m_currentmodule.name << ": unknown state: " << state << " @ " << std::hex << m_instream.tellg() << std::endl;
            return false;
        }

        if(!it->second())
        {
            std::cout << m_currentmodule.name << ": state: " << state << " failed @ " << std::hex << m_instream.tellg() << std::endl;
            return false;
        }

        if(state == PSYQState::SectionEOF)
            break;
    }

    return true;
}

bool PSYQLib::readAllModules()
{
    PSYQModule psyqmodule;

    while(m_instream)
    {
        size_t modulebase = m_instream.tellg();
        m_currentmodule = this->readModule();
        m_currentlink = &m_currentmodule.link;

        m_instream.seekg(modulebase + m_currentmodule.offsetlink, std::ios::beg); // Seek to LNK

        if(!m_instream)
            break;

        if(!this->readLink()) // Check fail state
        {
            m_modules.clear();
            return false;
        }

        m_modules.push_back(m_currentmodule);
    }

    return true;
}
