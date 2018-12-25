#include "json_generator.h"
#include <fstream>

JSONGenerator::JSONGenerator(): PatternGenerator() { }
std::string JSONGenerator::name() const { return "JSON Pattern"; }

bool JSONGenerator::generate(const std::string &infile, const std::string &prefix)
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

        if(!this->checkKey(patterns, "assembler", bytepattern.assembler) || !this->checkKey(patterns, "bits", bytepattern.bits) ||
                                                                            !this->checkKey(patterns, "pattern", bytepattern.pattern))
        {
            skipped++;
            continue;
        }


        if(!this->checkKey(patterns, "name", bytepattern.name) || !this->checkKey(patterns, "symboltype", bytepattern.symboltype) ||
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
