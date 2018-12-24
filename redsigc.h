#ifndef REDSIGC_H
#define REDSIGC_H

#define REDSIGC_VERSION std::string("0.5")

#include <functional>
#include <memory>
#include <string>
#include <deque>
#include <list>
#include <cxxopts.hpp>
#include <redasm/redasm_api.h>
#include "patterngenerator.h"

struct SelectedOptions
{
    SelectedOptions(): flags(0) { }

    int flags;
    std::vector<std::string> defaultargs;
    std::string prefix, symbol, infolder;

    bool has(int flag) const { return flags & flag; }
    const std::string& input() const { return defaultargs.front(); }
    const std::string& output() const { return defaultargs.back(); }
};

class REDSigC
{
    private:
        enum { None = 0, JSONOutput = 1, SDBOutput = 2, AutoPrefix = 4, Disassemble = 8, Folder = 16 };
        typedef std::function<PatternGenerator*(const std::string&, const std::string&)> GeneratorCallback;

    public:
        REDSigC();
        int run(int argc, char **argv);

    private:
        static std::string autoModuleName(std::string infile);
        PatternGenerator* getPatternGenerator(const std::string& infile, const std::string& prefix);
        void getInputFiles(std::list<std::string>& infiles) const;
        bool disassemblePattern(PatternGenerator* patterngenerator);
        bool checkOptions(int argc, char **argv);

    private:
        template<typename T> static bool checkOption(const cxxopts::ParseResult& parseresult, const std::string& option, T* result = NULL);
        template<typename T> PatternGenerator* generateCallback(const std::string& infile, const std::string& prefix);

    private:
        SelectedOptions m_options;
        std::list< std::unique_ptr<PatternGenerator> > m_activegenerators;
        std::list<GeneratorCallback> m_generators;
};

template<typename T> PatternGenerator* REDSigC::generateCallback(const std::string& infile, const std::string &prefix)
{
    if(m_options.input().empty())
        return NULL;

    std::unique_ptr<T> p = std::make_unique<T>();

    if(p->generate(infile, prefix))
        return p.release();

    return NULL;
}

template<typename T> bool REDSigC::checkOption(const cxxopts::ParseResult& parseresult, const std::string& option, T *result)
{
    try
    {
        if(!parseresult.count(option))
            return false;

        T opt = parseresult[option].as<T>();

        if(result)
            *result = opt;
    }
    catch(const cxxopts::OptionException& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    return true;
}

#endif // REDSIGC_H
