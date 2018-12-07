#include <iostream>
#include <redasm/database/signaturedb.h>
#include "psx/psyqlib_generator.h"

#define REDSIGC_VERSION 0.1

void showUsage()
{
    std::cout << "Usage REDSigC [options] inputfile [outputfile]" << std::endl;
    std::cout << "\t -s SDB Output (Default)" << std::endl;
    std::cout << "\t -j JSON Output" << std::endl;
}

int main(int argc, char* argv[])
{
    std::cout << "REDSigC Version " << REDSIGC_VERSION << std::endl;

    if(argc < 2)
    {
        showUsage();
        return 0;
    }

    std::string infile = argv[1], outfile = (argc > 2) ? argv[2] : infile;

    PsyQLibGenerator psyqgen;

    if(!psyqgen.generate(infile, "LIBAPI"))
    {
        std::cout << "ERROR: Cannot generate pattern for " << infile << std::endl;
        return 1;
    }

    if(!psyqgen.saveAsJSON(outfile))
    {
        std::cout << "ERROR: Cannot save pattern to " << outfile << std::endl;
        return 2;
    }

    return 0;
}
