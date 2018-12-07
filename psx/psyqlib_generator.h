#ifndef PSYQLIB_GENERATOR_H
#define PSYQLIB_GENERATOR_H

#include "../patterngenerator.h"

class PsyQLibGenerator: public PatternGenerator
{
    public:
        PsyQLibGenerator();
        virtual std::string name() const;
        virtual bool generate(const std::string& libfile, const std::string& prefix);
};

#endif // PSYQLIB_GENERATOR_H
