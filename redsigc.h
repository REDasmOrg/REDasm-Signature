#ifndef REDSIGC_H
#define REDSIGC_H

#define REDSIGC_VERSION 0.1

#include <functional>
#include <memory>
#include <string>
#include <list>
#include <redasm/redasm_api.h>
#include "generator/patterngenerator.h"

class REDSigC
{
    private:
        enum { None = 0, JSONOutput = 1, SDBOutput = 2 };
        typedef std::function<PatternGenerator*()> GeneratorCallback;

    public:
        REDSigC(int argc, char **argv);
        int run();

    private:
        template<typename T> PatternGenerator* generateCallback();
        int showUsage() const;
        bool checkOptions();

    private:
        int m_options, m_argc;
        char** m_argv;
        std::string m_prefix, m_infile, m_outfile;
        std::list<GeneratorCallback> m_generators;
};

template<typename T> PatternGenerator* REDSigC::generateCallback()
{
    std::unique_ptr<T> p = std::make_unique<T>();

    if(p->generate(m_infile, m_prefix))
        return p.release();

    return NULL;
}

#endif // REDSIGC_H
