#include "json_generator.h"
#include <fstream>

JSONGenerator::JSONGenerator(): PatternGenerator() { }
std::string JSONGenerator::name() const { return "JSON Pattern"; }

bool JSONGenerator::disassemble(const std::string &pattern)
{
    std::cout << pattern << std::endl;
    return true;
}

bool JSONGenerator::generate(const std::string &infile, const std::string &prefix)
{
    std::ifstream ifs(infile);

    if(!ifs.is_open())
        return false;

    json patterns;
    ifs >> patterns;

    if(patterns.empty())
        return false;

    int skipped = 0;

    for(const auto& pattern : patterns)
    {
        BytePattern bytepattern;
        auto it = pattern.find("pattern");

        if(it == pattern.end())
        {
            skipped++;
            continue;
        }

        bytepattern.pattern = *it;
        it = pattern.find("name");

        if(it == pattern.end())
        {
            skipped++;
            continue;
        }

        bytepattern.name = *it;
        it = pattern.find("name");

        if(it == pattern.end())
        {
            skipped++;
            continue;
        }

        bytepattern.symboltype = *it;

        if(!this->isBytePatternValid(bytepattern))
        {
            skipped++;
            continue;
        }

        this->push_back(bytepattern);
    }

    if(skipped > 0)
        std::cout << "WARNING: Skipped " << skipped << "/" << patterns.size() << std::endl;

    return !this->empty();
}
