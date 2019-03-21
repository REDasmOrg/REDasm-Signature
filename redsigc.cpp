#include "redsigc.h"
#include <iostream>
#include <fstream>
#include <dirent.h>
#include "generators/generators.h"

REDSigC::REDSigC()
{
    REDasm::Context::sync(true);
    REDasm::init({ });
    Generators::init();
}

int REDSigC::run(int argc, char **argv)
{
    if(!this->checkOptions(argc, argv))
        return 0;

    std::list<std::string> infiles;
    this->getInputFiles(infiles);

    if(infiles.empty())
    {
        std::cout << "ERROR: Cannot get input files" << std::endl;
        return 1;
    }

    PatternGenerator* patterngenerator = this->getPatternGenerator(infiles);

    if(!patterngenerator)
    {
        std::cout << "ERROR: Cannot find a valid Pattern generator" << std::endl;
        return 1;

    }
    else
        std::cout << "Using Pattern generator: " + REDasm::quoted(patterngenerator->name()) << " (" << patterngenerator->assembler() << ")" << std::endl;

    for(const std::string& infile : infiles)
    {
        if(!patterngenerator->test(infile))
        {
            std::cout << "[v] Skipping " << infile << std::endl;
            continue;
        }
        else
            std::cout << "[x] Parsing " << infile << std::endl;

        patterngenerator->generate(infile);

        if(m_options.has(REDSigC::Disassemble) && this->disassemblePattern(patterngenerator))
            return 0;
    }

    if(m_options.has(REDSigC::JSONSourceOutput))
    {
        json jsonsource = json::object();
        jsonsource["name"] = patterngenerator->name();
        jsonsource["assembler"] = patterngenerator->assembler();
        jsonsource["source_patterns"] = json::array();

        if(!patterngenerator->writePatternsSource(jsonsource["source_patterns"]))
        {
            std::cout << "ERROR: Cannot save JSON Source pattern(s)" << std::endl;
            return 2;
        }

        std::fstream fs(m_options.output(), std::ios::out | std::ios::trunc);

        if(!fs.is_open())
        {
            std::cout << "ERROR: Cannot write JSON Source file" << std::endl;
            return 2;
        }

        fs << jsonsource.dump(2);
    }
    else
    {
        REDasm::SignatureDB signaturedb;
        signaturedb.setAssembler(patterngenerator->assembler());

        if(!patterngenerator->writePatterns(signaturedb))
        {
            std::cout << "ERROR: Cannot save JSON pattern(s)" << std::endl;
            return 2;
        }

        if(!signaturedb.save(m_options.output()))
        {
            std::cout << "ERROR: Cannot write JSON file" << std::endl;
            return 2;
        }
    }

    return 0;
}

std::string REDSigC::autoModuleName(std::string infile)
{
    // Remove directory if present.
    // Do this before extension removal in case directory has a period character.
    size_t lastdirsepidx = infile.find_last_of(REDasm::Context::dirSeparator);

    if(lastdirsepidx != std::string::npos)
        infile.erase(0, lastdirsepidx + 1);

    // Remove extension if present.
    size_t dotidx = infile.rfind('.');

    if(dotidx != std::string::npos)
        infile.erase(dotidx);

    return infile;
}

PatternGenerator *REDSigC::getPatternGenerator(const std::list<std::string> &infiles)
{
    PatternGenerator* patterngenerator = nullptr;

    for(const std::string& infile : infiles)
    {
        if(patterngenerator)
            break;

        patterngenerator = Generators::getPattern(infile,
                                                  m_options.has(REDSigC::AutoPrefix) ? autoModuleName(infile) : m_options.prefix,
                                                  m_options.suffix, !m_options.has(REDSigC::Disassemble));
    }

    return patterngenerator;
}

void REDSigC::getInputFiles(std::list<std::string> &infiles) const
{
    dirent* de = NULL;

    if(m_options.has(REDSigC::Folder))
    {
        DIR* dir = opendir(m_options.input().c_str());

        if(!dir)
        {
            std::cout << "ERROR: Cannot open " << m_options.input().c_str() << std::endl;
            return;
        }

        while((de = readdir(dir)) != NULL)
        {
            if((de->d_name[0] == '.') || (de->d_type == DT_DIR))
                continue;

            infiles.push_back(REDasm::makePath(m_options.input(), de->d_name));
        }

        closedir(dir);
    }
    else
    {
        DIR* dir = opendir(m_options.input().c_str());

        if(dir)
            std::cout << "ERROR: Expected file, got a directory" << std::endl;
        else
            infiles.push_back(m_options.input());

        closedir(dir);
    }
}

bool REDSigC::disassemblePattern(PatternGenerator *patterngenerator)
{
    for(auto it = patterngenerator->begin(); it != patterngenerator->end(); it++)
    {
        if(it->name != m_options.symbol)
            continue;

        std::cout << "Listing of " << REDasm::quoted(it->name) << std::endl;
        std::replace(it->pattern.begin(), it->pattern.end(), WILDCARD_CHARACTER, 'F');
        patterngenerator->disassemble(*it);
        return true;
    }

    return false;
}

bool REDSigC::checkOptions(int argc, char **argv)
{
    cxxopts::Options options("redsigc", "REDasm Signature Compiler (Version " + REDSIGC_VERSION + ")");

    options.add_options("Output")
            ("j, jsonsource", "JSON Source Output")
            ("f, folder", "Input Folder", cxxopts::value<std::string>(), "path");

    options.add_options("Symbols")
            ("a, autoprefix", "Auto Prefix")
            ("d, disasm", "Disassemble Symbol", cxxopts::value<std::string>(), "symbol")
            ("p, prefix", "Prefix", cxxopts::value<std::string>(), "name")
            ("s, suffix", "Suffix", cxxopts::value<std::string>(), "name");

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
            m_options.flags |= REDSigC::JSONSourceOutput;

        if(REDSigC::checkOption(result, "f", &m_options.infolder))
        {
            m_options.flags |= REDSigC::Folder;
            m_options.defaultargs.insert(m_options.defaultargs.begin(), m_options.infolder);
            REDasm::Context::cwd(m_options.infolder);
        }

        if(REDSigC::checkOption<bool>(result, "a"))
            m_options.flags |= REDSigC::AutoPrefix;

        if(REDSigC::checkOption(result, "d", &m_options.symbol))
            m_options.flags |= REDSigC::Disassemble;

        REDSigC::checkOption(result, "p", &m_options.prefix);
        REDSigC::checkOption(result, "s", &m_options.suffix);
        return true;
    }

    std::cout << options.help({ "Output", "Symbols" }) << std::endl;
    return false;
}
