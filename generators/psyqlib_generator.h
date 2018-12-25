#ifndef PSYQLIB_GENERATOR_H
#define PSYQLIB_GENERATOR_H

#include "../patterngenerator.h"

class PsyQLibGenerator: public PatternGenerator
{
    public:
        PsyQLibGenerator();
        virtual std::string name() const;
        virtual bool disassemble(const std::string& pattern);
        virtual bool generate(const std::string& infile, const std::string& prefix);

    private:
        void stopAtDelaySlot(std::string& subpattern) const;
        void fixTail(std::string& subpattern) const;
};

#endif // PSYQLIB_GENERATOR_H
