#include "patterngenerator.h"
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

PatternGenerator::PatternGenerator(): std::list<BytePattern>() { }

bool PatternGenerator::saveAsJSON(const std::string &jsonfile)
{
    std::fstream fs(jsonfile + ".json", std::ios::out | std::ios::trunc);

    if(!fs.is_open())
        return false;

    auto patterns = json::array();

    for(auto it = this->begin(); it != this->end(); it++)
    {
        auto patternobj = json::object();
        patternobj["pattern"] = it->pattern;
        patternobj["names"] = json::array();

        for(auto& name : it->names)
        {
            auto nameobj = json::object();
            nameobj["offset"] = name.first;
            nameobj["name"] = name.second;

            patternobj["names"].push_back(nameobj);
        }

        patterns.push_back(patternobj);
    }

    fs << patterns.dump(2);
    return true;
}

void PatternGenerator::wildcard(BytePattern *bytepattern, size_t pos, size_t n)
{
    pos *= 2;

    if(pos >= bytepattern->pattern.size())
    {
        std::cout << "WARNING: " << pos << " >= " << bytepattern->pattern.size() << std::endl;
        return;
    }

    n *= 2;
    bytepattern->pattern.replace(pos, n, WILDCARD_PATTERN);
}

std::string PatternGenerator::fullname(const std::string &prefix, const std::string &name) { return prefix + "." + name; }
