#ifndef JSON_GENERATOR_H
#define JSON_GENERATOR_H

#include "../patterngenerator.h"

class JSONGenerator: public PatternGenerator
{
    public:
        JSONGenerator();
        virtual std::string name() const;
        virtual std::string assembler() const;
        virtual bool test(const std::string& infile);
        virtual void generate(const std::string& infile);

    private:
        template<typename T> bool checkKey(const json& pattern, const std::string& key, T& result) const;

    private:
        std::string m_name, m_assembler;
};

template<typename T> bool JSONGenerator::checkKey(const json &pattern, const std::string& key, T& result) const
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
