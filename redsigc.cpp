#include "redsigc.h"
#include <redasm/database/signaturedb.h>
#include <redasm/support/utils.h>
#include <iostream>
#include <cstring>

#include "../psx/psyqlib_generator.h"

#define REGISTER_GENERATOR(T) m_generators.push_back([&]() -> PatternGenerator* { return this->generateCallback<T>(); })

REDSigC::REDSigC(int argc, char **argv): m_options(REDSigC::None), m_argc(argc), m_argv(argv)
{
    REGISTER_GENERATOR(PsyQLibGenerator);
}

int REDSigC::run()
{
    std::cout << "REDSigC Version " << REDSIGC_VERSION << std::endl;

    if((m_argc < 2) || !this->checkOptions())
        return this->showUsage();

    std::unique_ptr<PatternGenerator> patterngenerator;

    for(auto it = m_generators.begin(); it != m_generators.end(); it++)
    {
        patterngenerator = std::unique_ptr<PatternGenerator>((*it)());

        if(!patterngenerator)
            continue;

        std::cout << "Patterns generated with " << REDasm::quoted(patterngenerator->name()) << std::endl;

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

    std::cout << "No Pattern Generator found for " << m_infile << std::endl;
    return 0;
}

int REDSigC::showUsage() const
{
    std::cout << "Usage REDSigC [options] inputfile [outputfile]" << std::endl;

    std::cout << "Output:" << std::endl;
    std::cout << "\t -s SDB Output (Default)" << std::endl;
    std::cout << "\t -j JSON Output" << std::endl;

    std::cout << std::endl << "Symbols:" << std::endl;
    std::cout << "\t -p Symbol Prefix" << std::endl;

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
            if(i == (m_argc - 1))
                return false;

            i++;
            char* prefixarg = m_argv[++i];

            if(prefixarg[0] == '-')
                return false;

            m_prefix = prefixarg;
        }
        else if(m_argv[i][0] != '-')
        {
            if(m_infile.empty())
                m_infile = m_argv[i];
            else if(m_outfile.empty())
                m_outfile = m_argv[i];
        }
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
        m_outfile = m_infile + ((m_options & REDSigC::JSONOutput) ? ".json" : ".sdb");

    return !m_infile.empty();
}
