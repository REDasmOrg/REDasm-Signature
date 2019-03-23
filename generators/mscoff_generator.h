#ifndef MSCOFF_GENERATOR_H
#define MSCOFF_GENERATOR_H

#include <redasm/loaders/mscoff/mscoff.h>
#include "../patterngenerator.h"

class MSCOFFGenerator : public PatternGenerator
{
    public:
        MSCOFFGenerator();
        virtual std::string name() const;
        virtual std::string assembler() const;
        virtual bool test(const std::string& infile);
        virtual void generate(const std::string& infile);

    private:
        std::unique_ptr<REDasm::MSCOFFLoader> m_loader;
};

#endif // MSCOFF_GENERATOR_H
