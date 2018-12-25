#ifndef JSON_GENERATOR_H
#define JSON_GENERATOR_H

#include "../patterngenerator.h"

class JSONGenerator: public PatternGenerator
{
    public:
        JSONGenerator();
        virtual std::string name() const;
        virtual bool generate(const std::string& infile, const std::string& prefix);

    private:
        template<typename T> bool checkKey(json& pattern, const std::string& key, T& result) const;
};

template<typename T> bool JSONGenerator::checkKey(json& pattern, const std::string& key, T& result) const
{
    auto it = pattern.find(key);

    if(it != pattern.end())
    {
        result = *it;
        return true;
    }

    return false;
}

#endif // JSON_GENERATOR_H
