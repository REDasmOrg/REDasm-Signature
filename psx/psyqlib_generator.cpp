#include "psyqlib_generator.h"
#include "psyqlib.h"
#include "../listingconsolerenderer.h"
#include <redasm/support/utils.h>
#include <iostream>
#include <map>

#define JR_RA "0800E003"
#define ADDIU "1800BD27"
#define NOP   "00000000"
#define MIPS_INSTRUCTION_SIZE_HEX (4 * 2)

PsyQLibGenerator::PsyQLibGenerator(): PatternGenerator() { }
std::string PsyQLibGenerator::name() const { return "PsyQ Lib Generator"; }

bool PsyQLibGenerator::disassemble(const std::string& pattern)
{
    REDasm::Buffer buffer = REDasm::bytes(pattern);
    std::unique_ptr<REDasm::Disassembler> disassembler(this->createDisassembler("mips32", 32, buffer));

    if(!disassembler)
        return false;

    std::unique_ptr<REDasm::Printer> printer(disassembler->createPrinter());
    disassembler->disassemble();

    ListingConsoleRenderer consolerenderer(disassembler.get());
    consolerenderer.renderAll();
    return true;
}

bool PsyQLibGenerator::generate(const std::string &infile, const std::string& prefix)
{
    PSYQLib psyqlib(infile);

    if(!psyqlib.load())
        return false;

    for(auto& psyqmodule : psyqlib.modules())
    {
        const PSYQLink& psyqlink = psyqmodule.link;

        std::unordered_map<u16, std::string> patterns;
        std::map<u32, std::string> offsets;

        for(auto& psyqsection: psyqlink.sections)
        {
            const auto& code = psyqsection.second.code;

            if(code.empty())
                continue;

            patterns[psyqsection.second.symbolnumber] = REDasm::hexstring(reinterpret_cast<const char*>(code.data()), code.size());
        }

        for(auto& psyqpatch : psyqlink.patches)
        {
            auto it = patterns.find(psyqpatch.patchsection);

            if(it == patterns.end())
                continue;

            wildcard(it->second, psyqpatch.offset, 2);
        }

        // Prepare offsets
        for(auto& psyqdefinition: psyqlink.definitions)
            offsets[psyqdefinition.second.offset] = psyqdefinition.second.name;

        for(auto& psyqdefinition: psyqlink.definitions)
        {
            if(psyqlink.sections.at(psyqdefinition.second.sectionnumber).name != ".text")
                continue;

            const std::string& pattern = patterns[psyqdefinition.second.sectionnumber];
            u64 length = pattern.size() - (psyqdefinition.second.offset * 2);

            auto it = offsets.find(psyqdefinition.second.offset);
            it++;

            if(it != offsets.end())
                length = (it->first - psyqdefinition.second.offset) * 2;

            std::string subpattern = this->subPattern(pattern, psyqdefinition.second.offset * 2, length);
            this->fixAddiu(subpattern);
            this->stopAtDelaySlot(subpattern);
            this->pushPattern(fullname(prefix, psyqdefinition.second.name), subpattern, REDasm::SymbolTypes::Function);
        }
    }

    return true;
}

void PsyQLibGenerator::stopAtDelaySlot(std::string &subpattern) const
{
    size_t pos = subpattern.rfind(JR_RA);

    if(pos == std::string::npos)
        return;

    if((subpattern.size() - pos) == MIPS_INSTRUCTION_SIZE_HEX)
        subpattern += NOP;
    else
        subpattern = subpattern.substr(0, pos + (MIPS_INSTRUCTION_SIZE_HEX * 2));
}

void PsyQLibGenerator::fixAddiu(std::string &subpattern) const
{
    std::string lastinstruction = subpattern.substr(subpattern.size() - MIPS_INSTRUCTION_SIZE_HEX);

    if(lastinstruction != ADDIU)
        return;

    size_t pos = subpattern.rfind(JR_RA);
    subpattern.insert(pos, ADDIU).replace(subpattern.size() - MIPS_INSTRUCTION_SIZE_HEX, MIPS_INSTRUCTION_SIZE_HEX, NOP);
}
