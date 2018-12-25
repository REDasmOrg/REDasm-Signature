#include "json_generator.h"
#include <fstream>

JSONGenerator::JSONGenerator(): PatternGenerator() { }
std::string JSONGenerator::name() const { return "JSON Pattern"; }

bool JSONGenerator::generate(const std::string &infile)
{
    std::ifstream ifs(infile);

    if(!ifs.is_open())
        return false;

    json patterns;

    try {
        ifs >> patterns;
    }
    catch(std::invalid_argument&) {
        return false;
    }

    if(patterns.empty())
        return false;

    int skipped = 0;

    for(const auto& pattern : patterns)
    {
        BytePattern bytepattern;

        if(!this->checkKey(pattern, "assembler", bytepattern.assembler) || !this->checkKey(pattern, "bits", bytepattern.bits) ||
                                                                           !this->checkKey(pattern, "pattern", bytepattern.pattern))
        {
            skipped++;
            continue;
        }


        if(!this->checkKey(pattern, "name", bytepattern.name) || !this->checkKey(pattern, "symboltype", bytepattern.symboltype) ||
                                                                 !this->isBytePatternValid(bytepattern))
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
