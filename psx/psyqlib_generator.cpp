#include "psyqlib_generator.h"
#include "psyqlib.h"
#include <redasm/support/utils.h>
#include <iostream>
#include <map>

PsyQLibGenerator::PsyQLibGenerator(): PatternGenerator() { }
std::string PsyQLibGenerator::name() const { return "PsyQ Lib Generator"; }

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
            std::cout << "Generating " << REDasm::quoted(psyqdefinition.second.name) << std::endl;
            const std::string& pattern = patterns[psyqdefinition.second.sectionnumber];
            u64 length = pattern.size();

            auto it = offsets.find(psyqdefinition.second.offset);
            it++;

            if(it != offsets.end())
                length = (psyqdefinition.second.offset + it->first) * 2;

            this->pushPattern(fullname(prefix, psyqdefinition.second.name),
                              this->subPattern(pattern, psyqdefinition.second.offset * 2, length),
                              REDasm::SymbolTypes::Function);
        }
    }

    return true;
}
