#ifndef JSON_GENERATOR_H
#define JSON_GENERATOR_H

#include "../patterngenerator.h"

class JSONGenerator: public PatternGenerator
{
    public:
        JSONGenerator();
        virtual std::string name() const;
        virtual bool disassemble(const std::string& pattern);
        virtual bool generate(const std::string& infile, const std::string& prefix);
};

#endif // JSON_GENERATOR_H
