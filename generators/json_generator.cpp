#include "json_generator.h"
#include <fstream>

JSONGenerator::JSONGenerator(): PatternGenerator() { }
std::string JSONGenerator::name() const { return m_name; }
std::string JSONGenerator::assembler() const { return m_assembler; }

bool JSONGenerator::test(const std::string &infile)
{
    std::ifstream ifs(infile);

    if(!ifs.is_open())
        return false;

    json jsonsource;

    try {
        ifs >> jsonsource;
    }
    catch(...) {
        return false;
    }

    if(jsonsource.empty())
        return false;

    if(jsonsource.contains("name"))
        m_name = jsonsource["name"];
    else
        return false;

    if(jsonsource.contains("assembler"))
        m_assembler = jsonsource["assembler"];
    else
        return false;

    return jsonsource.contains("source_patterns");
}

void JSONGenerator::generate(const std::string &infile)
{
    std::ifstream ifs(infile);

    if(!ifs.is_open())
        return;

    json jsonsource;
    ifs >> jsonsource;

    m_name = jsonsource["name"];
    m_assembler = jsonsource["assembler"];

    int skipped = 0;

    for(const auto& pattern : jsonsource["source_patterns"])
    {
        BytePattern bytepattern;

        if(!this->checkKey(pattern, "name", bytepattern.name) || !this->checkKey(pattern, "pattern", bytepattern.pattern))
        {
            skipped++;
            continue;
        }


        if(!this->checkKey(pattern, "symboltype", bytepattern.symboltype) || !this->isBytePatternValid(bytepattern))
        {
            skipped++;
            continue;
        }

        this->push_back(bytepattern);
    }

    if(skipped > 0)
        std::cout << "WARNING: Skipped " << skipped << "/" << jsonsource.size() << std::endl;
}
