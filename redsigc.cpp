#include "redsigc.h"
#include <redasm/database/signaturedb.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <dirent.h>
#include "../psx/psyqlib_generator.h"

#define DEFAULT_SIGNATURE_OUTPUT "signature"
#define REGISTER_GENERATOR(T)    m_generators.push_back([&](const std::string& infile, const std::string& prefix) -> PatternGenerator* { \
                                    return this->generateCallback<T>(infile, prefix); \
                                 })

REDSigC::REDSigC()
{
    REDasm::Runtime::syncMode(true);
    REDasm::init();

    REGISTER_GENERATOR(PsyQLibGenerator);
}

int REDSigC::run(int argc, char **argv)
{
    if(!this->checkOptions(argc, argv))
        return 0;

    std::list<std::string> infiles;
    this->getInputFiles(infiles);

    if(infiles.empty())
    {
        std::cout << "Cannot get input files" << std::endl;
        return 1;
    }

    for(const std::string& infile : infiles)
    {
        auto patterngenerator = this->getPatternGenerator(infile, m_options.has(REDSigC::AutoPrefix) ? autoModuleName(infile) : m_options.prefix);

        if(!patterngenerator)
        {
            if(!m_options.has(REDSigC::Disassemble))
                std::cout << "Skipping " << infile << std::endl;

            continue;
        }

        if(m_options.has(REDSigC::Disassemble) && this->disassemblePattern(patterngenerator))
            return 0;
    }

    if(m_options.has(REDSigC::JSONOutput))
    {
        json patterns = json::array();

        for(auto it = m_activegenerators.begin(); it != m_activegenerators.end(); it++)
        {
            if((*it)->saveAsJSON(patterns))
                continue;

            std::cout << "ERROR: Cannot save JSON pattern(s)" << std::endl;
            return 2;
        }

        std::fstream fs(m_options.output() + ".json", std::ios::out | std::ios::trunc);

        if(!fs.is_open())
        {
            std::cout << "ERROR: Cannot write JSON file" << std::endl;
            return 2;
        }

        fs << patterns.dump(2);
    }
    else if(m_options.has(REDSigC::SDBOutput))
    {
        REDasm::SignatureDB signaturedb;

        for(auto it = m_activegenerators.begin(); it != m_activegenerators.end(); it++)
        {
            if((*it)->saveAsSDB(signaturedb))
                continue;

            std::cout << "ERROR: Cannot save SDB pattern(s)" << std::endl;
            return 2;
        }

        if(!signaturedb.save(m_options.output() + ".sdb"))
        {
            std::cout << "ERROR: Cannot write SDB file" << std::endl;
            return 2;
        }
    }

    return 0;
}

std::string REDSigC::autoModuleName(std::string infile)
{
    // Remove directory if present.
    // Do this before extension removal in case directory has a period character.
    size_t lastdirsepidx = infile.find_last_of(REDasm::Runtime::rntDirSeparator);

    if(lastdirsepidx != std::string::npos)
        infile.erase(0, lastdirsepidx + 1);

    // Remove extension if present.
    size_t dotidx = infile.rfind('.');

    if(dotidx != std::string::npos)
        infile.erase(dotidx);

    return infile;
}

PatternGenerator *REDSigC::getPatternGenerator(const std::string &infile, const std::string& prefix)
{
    for(auto it = m_activegenerators.begin(); it != m_activegenerators.end(); it++)
    {
        if(!(*it)->generate(infile, prefix))
            continue;

        if(!m_options.has(REDSigC::Disassemble))
            std::cout << (*it)->name() << ": " << infile << std::endl;

        return it->get();
    }

    for(auto it = m_generators.begin(); it != m_generators.end(); it++)
    {
        PatternGenerator* patterngenerator = (*it)(infile, prefix);

        if(!patterngenerator)
            continue;

        m_activegenerators.emplace_back(patterngenerator);
        return m_activegenerators.back().get();
    }

    return NULL;
}

void REDSigC::getInputFiles(std::list<std::string> &infiles) const
{
    if(m_options.has(REDSigC::Folder))
    {
        DIR* dir = opendir(m_options.input().c_str());

        if(!dir)
        {
            std::cout << "Cannot open " << m_options.input().c_str() << std::endl;
            return;
        }

        dirent* de = NULL;

        while((de = readdir(dir)) != NULL)
        {
            if((de->d_name[0] == '.') || !std::strcmp(de->d_name, ".") || !std::strcmp(de->d_name, ".."))
                continue;

            infiles.push_back(REDasm::makePath(m_options.input(), de->d_name));
        }

        closedir(dir);
    }
    else
        infiles.push_back(m_options.input());
}

bool REDSigC::disassemblePattern(PatternGenerator *patterngenerator)
{
    for(auto it = patterngenerator->begin(); it != patterngenerator->end(); it++)
    {
        if(it->name != m_options.symbol)
            continue;

        std::cout << "Listing of " << REDasm::quoted(it->name) << std::endl;
        std::replace(it->pattern.begin(), it->pattern.end(), WILDCARD_CHARACTER, 'F');
        patterngenerator->disassemble(it->pattern);
        return true;
    }

    return false;
}

bool REDSigC::checkOptions(int argc, char **argv)
{
    cxxopts::Options options("redsigc", "REDasm Signature Compiler (Version " + REDSIGC_VERSION + ")");

    options.add_options("Output")
            ("s, sdb", "SDB Output")
            ("j, json", "JSON Output")
            ("f, folder", "Input Folder", cxxopts::value<std::string>(), "path");

    options.add_options("Symbols")
            ("a, autoprefix", "Auto Prefix")
            ("d, disasm", "Disassemble Symbol", cxxopts::value<std::string>(), "symbol")
            ("p, prefix", "Prefix", cxxopts::value<std::string>(), "name");

    options.add_options()
            ("defaultargs", std::string(), cxxopts::value< std::vector<std::string> >());

    options.positional_help("inputfile outputfile")
           .parse_positional("defaultargs");

    int optionargc = argc; // Keep 'argc' value
    auto result = options.parse(optionargc, argv);

    if(argc > 1)
    {
        REDSigC::checkOption(result, "defaultargs", &m_options.defaultargs);

        if(REDSigC::checkOption<bool>(result, "j"))
            m_options.flags |= REDSigC::JSONOutput;
        else
            m_options.flags |= REDSigC::SDBOutput;

        if(REDSigC::checkOption(result, "f", &m_options.infolder))
        {
            m_options.flags |= REDSigC::Folder;
            m_options.defaultargs.insert(m_options.defaultargs.begin(), m_options.infolder);
        }

        if(REDSigC::checkOption<bool>(result, "a"))
            m_options.flags |= REDSigC::AutoPrefix;

        if(REDSigC::checkOption(result, "d", &m_options.symbol))
            m_options.flags |= REDSigC::Disassemble;

        REDSigC::checkOption(result, "p", &m_options.prefix);
        return true;
    }

    std::cout << options.help({ "Output", "Symbols" }) << std::endl;
    return false;
}
