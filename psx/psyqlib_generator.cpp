#include "psyqlib_generator.h"
#include "psyqlib.h"
#include <redasm/support/utils.h>
#include <iostream>

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

        std::unordered_map<u16, BytePattern> bytepatterns;

        for(auto& psyqsection: psyqlink.sections)
        {
            const auto& code = psyqsection.second.code;

            if(code.empty())
                continue;

            BytePattern bp;
            bp.pattern = REDasm::hexstring(reinterpret_cast<const char*>(code.data()), code.size());
            bytepatterns[psyqsection.second.symbolnumber] = bp;
        }

        for(auto& psyqdefinition: psyqlink.definitions)
        {
            std::cout << "Generating " << REDasm::quoted(psyqdefinition.second.name) << std::endl;
            BytePattern& bp = bytepatterns[psyqdefinition.second.sectionnumber];

            bp.names.push_back(std::make_pair(psyqdefinition.second.offset,
                                              fullname(prefix, psyqdefinition.second.name)));
        }

        for(auto& psyqpatch : psyqlink.patches)
        {
            auto it = bytepatterns.find(psyqpatch.patchsection);

            if(it == bytepatterns.end())
                continue;

            wildcard(&it->second, psyqpatch.offset, 1);
        }

        for(auto& item : bytepatterns)
            this->push_back(item.second);
    }

    return true;
}
