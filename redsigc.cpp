#include "redsigc.h"
#include <redasm/database/signaturedb.h>
#include <redasm/support/utils.h>
#include <redasm/redasm_runtime.h>
#include <iostream>
#include <cstring>
#include "../psx/psyqlib_generator.h"

#define DEFAULT_SIGNATURE_OUTPUT "signature"
#define REGISTER_GENERATOR(T)    m_generators.push_back([&]() -> PatternGenerator* { return this->generateCallback<T>(); })

REDSigC::REDSigC(int argc, char **argv): m_options(REDSigC::None), m_argc(argc), m_argv(argv)
{
    REDasm::Runtime::syncMode(true);
    REDasm::init();
    REGISTER_GENERATOR(PsyQLibGenerator);
}

int REDSigC::run()
{
    std::cout << "REDasm Signature Compiler (Version " << REDSIGC_VERSION << ")" << std::endl;

    if((m_argc < 2) || !this->checkOptions())
        return this->showUsage();

    std::unique_ptr<PatternGenerator> patterngenerator;

    for(auto it = m_generators.begin(); it != m_generators.end(); it++)
    {
        patterngenerator = std::unique_ptr<PatternGenerator>((*it)());

        if(!patterngenerator)
            continue;

        std::cout << "Patterns generated with " << REDasm::quoted(patterngenerator->name()) << std::endl;

        for(int i = 1; i < m_infiles.size(); i++) // First will be generated by the callback
        {
            if(patterngenerator->generate(this->inputFile(m_infiles[i]), (m_options & REDSigC::AutoPrefix) ? autoModuleName(m_infiles[i]) : m_prefix))
                continue;

            std::cout << "Invalid file " << m_infiles[i] << std::endl;
            return 3;
        }

        if(m_options & REDSigC::Disassemble)
        {
            for(auto it = patterngenerator->begin(); it != patterngenerator->end(); it++)
            {
                if(it->name != m_disassemblesymbol)
                    continue;

                std::cout << "Listing of " << REDasm::quoted(it->name) << std::endl;
                std::string pattern = it->pattern;
                std::replace(pattern.begin(), pattern.end(), WILDCARD_CHARACTER, 'F');
                patterngenerator->disassemble(pattern);
                break;
            }

            return 0;
        }

        if((m_options & REDSigC::JSONOutput) && !patterngenerator->saveAsJSON(m_outfile))
        {
            std::cout << "ERROR: Cannot save JSON pattern to " << m_outfile << std::endl;
            return 2;
        }
        else if((m_options & REDSigC::SDBOutput) && !patterngenerator->saveAsSDB(m_outfile))
        {
            std::cout << "ERROR: Cannot save SDB pattern to " << m_outfile << std::endl;
            return 2;
        }

        return 0;
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

std::string REDSigC::inputFile(const std::string &filename) const
{
    std::string infile = filename;

    if(!m_infolder.empty())
        infile = m_infolder + REDasm::Runtime::rntDirSeparator + infile;

    return infile;
}

int REDSigC::showUsage() const
{
    std::cout << "Usage redsigc [options] inputfiles" << std::endl;

    std::cout << "Output:" << std::endl;
    std::cout << "\t -s SDB Output (Default)" << std::endl;
    std::cout << "\t -j JSON Output" << std::endl;
    std::cout << "\t -o [filename] Output File" << std::endl;
    std::cout << "\t -i [folder] Input Folder" << std::endl;

    std::cout << std::endl << "Symbols:" << std::endl;
    std::cout << "\t -p [prefix]  Symbol Prefix" << std::endl;
    std::cout << "\t -d [symbol]  Symbol Prefix" << std::endl;
    std::cout << "\t -ap Auto Prefix" << std::endl;

    std::cout << std::endl;
    return 0;
}

bool REDSigC::checkOptions()
{
    int i = 1;

    while(i < m_argc)
    {
        if(!std::strcmp(m_argv[i], "-p"))
        {
            if(!this->checkOptionArg(m_prefix, i))
                return false;
        }
        else if(!std::strcmp(m_argv[i], "-o"))
        {
            if(!this->checkOptionArg(m_outfile, i))
                return false;
        }
        else if(!std::strcmp(m_argv[i], "-i"))
        {
            if(!this->checkOptionArg(m_infolder, i))
                return false;
        }
        else if(!std::strcmp(m_argv[i], "-of"))
        {
            if(!this->checkOptionArg(m_outfolder, i))
                return false;
        }
        else if(m_argv[i][0] != '-')
        {
            if(i < m_argc)
                m_infiles.push_back(m_argv[i]);
        }
        else if(!std::strcmp(m_argv[i], "-d"))
        {
            if(!this->checkOptionArg(m_disassemblesymbol, i))
                return false;

            m_options |= REDSigC::Disassemble;
        }
        else if(!std::strcmp(m_argv[i], "-ap"))
            m_options |= REDSigC::AutoPrefix;
        else if(!std::strcmp(m_argv[i], "-s"))
            m_options |= REDSigC::SDBOutput;
        else if(!std::strcmp(m_argv[i], "-j"))
            m_options |= REDSigC::JSONOutput;

        if((m_options & REDSigC::SDBOutput) && (m_options & REDSigC::JSONOutput))
            return false;

        i++;
    }

    if(!(m_options & REDSigC::SDBOutput) && !(m_options & REDSigC::JSONOutput))
        m_options |= REDSigC::SDBOutput;

    if(m_outfile.empty())
        m_outfile = DEFAULT_SIGNATURE_OUTPUT;

    return !m_infiles.empty();
}

bool REDSigC::checkOptionArg(std::string &arg, int &i) const
{
    i++;

    if((i >= m_argc) || (m_argv[i][0] == '-'))
        return false;

    arg = m_argv[i];
    return true;
}
