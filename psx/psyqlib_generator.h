#ifndef PSYQLIB_GENERATOR_H
#define PSYQLIB_GENERATOR_H

#include "../patterngenerator.h"

class PsyQLibGenerator: public PatternGenerator
{
    public:
        PsyQLibGenerator();
        virtual std::string name() const;
        virtual bool generate(const std::string& libfile, const std::string& prefix);

    private:
        void truncateAtDelaySlot(std::string& subpattern) const;
};

#endif // PSYQLIB_GENERATOR_H
