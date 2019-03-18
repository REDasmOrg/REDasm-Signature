#include "patterngenerator.h"
#include "listingconsolerenderer.h"
#include <redasm/support/utils.h>
#include <redasm/support/hash.h>
#include <redasm/plugins/plugins.h>
#include <redasm/loaders/binary/binary.h>
#include <fstream>

#define PATTERN_OFFSET(offset)  (offset / 2)
#define PATTERN_SIZE(size)      PATTERN_OFFSET(size)
#define SIGNATURE_BYTES(pattern) PATTERN_SIZE(pattern.size())

PatternGenerator::PatternGenerator(): std::list<BytePattern>() { }
void PatternGenerator::setPrefix(const std::string &prefix) { m_prefix = prefix; }
void PatternGenerator::setSuffix(const std::string &suffix) { m_suffix = suffix; }

bool PatternGenerator::saveAsJSON(json &patterns)
{
    for(auto it = this->begin(); it != this->end(); it++)
    {
        if(!this->isBytePatternValid(*it))
            continue;

        auto patternobj = json::object();
        patternobj["name"] = it->name;
        patternobj["pattern"] = it->pattern;
        patternobj["assembler"] = it->assembler;
        patternobj["bits"] = it->bits;
        patternobj["symboltype"] = it->symboltype;
        patterns.push_back(patternobj);
    }

    return true;
}

bool PatternGenerator::saveAsSDB(REDasm::SignatureDB &sigdb)
{
    for(auto it = this->begin(); it != this->end(); it++)
    {
        if(!this->isBytePatternValid(*it))
            continue;

        REDasm::Signature sig;
        sig.bits = it->bits;
        sig.symboltype = it->symboltype;
        sig.size = SIGNATURE_BYTES(it->pattern);
        sig.name = it->name;
        sig.assembler = it->assembler;

        if(!this->appendAllPatterns(&sig, *it))
            return false;

        sigdb << sig;
    }

    return true;
}

bool PatternGenerator::disassemble(const BytePattern& bytepattern)
{
    REDasm::MemoryBuffer buffer = REDasm::bytes(bytepattern.pattern);
    std::unique_ptr<REDasm::Disassembler> disassembler(this->createDisassembler(bytepattern.assembler.c_str(), bytepattern.bits, &buffer));

    if(!disassembler)
        return false;

    std::unique_ptr<REDasm::Printer> printer(disassembler->createPrinter());
    disassembler->disassemble();

    ListingConsoleRenderer consolerenderer(disassembler.get());
    consolerenderer.renderAll();
    return true;
}

bool PatternGenerator::appendAllPatterns(REDasm::Signature *signature, const BytePattern &bytepattern) const
{
    int offset = 0;

    while(offset < bytepattern.pattern.size())
    {
        REDasm::SignaturePattern sigpatt;

        bool wildcard = false;
        std::string chunk = this->getChunk(bytepattern.pattern, offset, &wildcard);

        if(chunk.empty())
        {
            std::cout << "ERROR: Empty chunk @ " << REDasm::hex(offset) << std::endl;
            return false;
        }

        sigpatt.size = PATTERN_SIZE(chunk.size());

        if(!wildcard)
        {
            sigpatt.type = REDasm::SignaturePatternType::CheckSum;
            sigpatt.checksum = this->chunkChecksum(chunk);
        }
        else
            sigpatt.type = REDasm::SignaturePatternType::Skip;

        offset += chunk.size();
        signature->patterns.push_back(sigpatt);
    }

    return true;
}

bool PatternGenerator::isBytePatternValid(const BytePattern &bytepattern) const
{
    if(bytepattern.pattern.empty())
        return false;

    if(bytepattern.name.empty())
    {
        std::cout << "WARNING: Skipping anonymous pattern" << std::endl;
        return false;
    }

    int wc = 0;

    for(char ch : bytepattern.pattern)
    {
        if(ch == WILDCARD_CHARACTER)
            wc++;
    }

    if((wc / static_cast<float>(bytepattern.pattern.size())) >= 0.50)
    {
        std::cout << "WARNING: Skipping unrealiable pattern: " << REDasm::quoted(bytepattern.name) << std::endl;
        return false;
    }

    return true;
}

u16 PatternGenerator::chunkChecksum(const std::string &chunk) const
{
    std::vector<u8> bytes(SIGNATURE_BYTES(chunk));

    for(size_t i = 0, j = 0; i < chunk.size(); i += 2, j++)
        REDasm::byte(chunk, &bytes[j], i);

    return REDasm::Hash::crc16(bytes.data(), bytes.size());
}

std::string PatternGenerator::getChunk(const std::string &s, int offset, bool* wildcard) const
{
    std::string chunk, hexbyte = s.substr(offset, 2);

    if(hexbyte == WILDCARD_PATTERN)
    {
        *wildcard = true;

        while((offset < s.size()) && (hexbyte == WILDCARD_PATTERN))
        {
            chunk += hexbyte;
            offset += 2;
            hexbyte = s.substr(offset, 2);
        }
    }
    else
    {
        *wildcard = false;

        while((offset < s.size()) && (hexbyte != WILDCARD_PATTERN))
        {
            chunk += hexbyte;
            offset += 2;
            hexbyte = s.substr(offset, 2);
        }
    }

    return chunk;
}

std::string PatternGenerator::fullName(const std::string &name) const
{
    std::string fullname = name;

    if(!m_prefix.empty())
        fullname = m_prefix + "." + fullname;

    return fullname + m_suffix;
}

REDasm::Disassembler *PatternGenerator::createDisassembler(const std::string& assemblerid, u32 bits, REDasm::AbstractBuffer *buffer)
{
    if(buffer->empty())
    {
        std::cout << "ERROR: Pattern is empty" << std::endl;
        return NULL;
    }

    const REDasm::AssemblerPlugin_Entry* assemblerentry = REDasm::getAssembler(assemblerid);

    if(!assemblerentry)
    {
        std::cout << "ERROR: Invalid Assembler: " << REDasm::quoted(assemblerid) << std::endl;
        return NULL;
    }

    address_t baseaddress = 1u << (bits - 1);
    REDasm::BinaryLoader* loader = new REDasm::BinaryLoader(buffer);
    loader->build(assemblerentry->name(), 0, baseaddress, baseaddress);
    return new REDasm::Disassembler(assemblerentry->init(), loader); // Takes ownership
}

void PatternGenerator::pushPattern(const std::string &name, const std::string &pattern, const std::string& assembler, u32 bits, u32 symboltype)
{
    this->push_back({ this->fullName(name), pattern, assembler, bits, symboltype });
}

std::string PatternGenerator::subPattern(const std::string &pattern, size_t pos, size_t len) { return pattern.substr(pos, len); }

void PatternGenerator::wildcard(std::string &pattern, size_t pos, size_t n)
{
    pos *= 2;

    if(pos >= pattern.size())
    {
        std::cout << "WARNING: " << pos << " >= " << pattern.size() << std::endl;
        return;
    }

    for(size_t i = 0; i < n; i++, pos += 2)
        pattern.replace(pos, 2, WILDCARD_PATTERN);
}
