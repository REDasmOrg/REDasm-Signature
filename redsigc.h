#ifndef REDSIGC_H
#define REDSIGC_H

#define REDSIGC_VERSION 0.1

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <redasm/redasm_api.h>
#include "patterngenerator.h"

class REDSigC
{
    private:
        enum { None = 0, JSONOutput = 1, SDBOutput = 2, AutoPrefix = 4, Disassemble = 8 };
        typedef std::function<PatternGenerator*()> GeneratorCallback;

    public:
        REDSigC(int argc, char **argv);
        int run();

    private:
        template<typename T> PatternGenerator* generateCallback();
        static std::string autoModuleName(std::string infile);
        std::string inputFile(const std::string& filename) const;
        int showUsage() const;
        bool checkOptions();
        bool checkOptionArg(std::string& arg, int& i) const;

    private:
        int m_options, m_argc;
        char** m_argv;
        std::string m_prefix, m_outfile, m_infolder, m_outfolder, m_disassemblesymbol;
        std::vector<std::string> m_infiles;
        std::list<GeneratorCallback> m_generators;
};

template<typename T> PatternGenerator* REDSigC::generateCallback()
{
    if(m_infiles.empty())
        return NULL;

    std::unique_ptr<T> p = std::make_unique<T>();
    p->setOutputFolder(m_outfolder);

    if(p->generate(this->inputFile(m_infiles[0]), (m_options & REDSigC::AutoPrefix) ? autoModuleName(m_infiles[0]) : m_prefix))
        return p.release();

    return NULL;
}

#endif // REDSIGC_H
