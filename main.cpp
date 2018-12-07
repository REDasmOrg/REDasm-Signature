#include <iostream>
#include "psx/psyqlib_generator.h"
#include "redsigc.h"

int main(int argc, char* argv[])
{
    REDSigC redsigc(argc, argv);
    return redsigc.run();

    std::string infile = argv[1], outfile = (argc > 2) ? argv[2] : infile;

    PsyQLibGenerator psyqgen;

    if(!psyqgen.saveAsJSON(outfile))
    {
        std::cout << "ERROR: Cannot save pattern to " << outfile << std::endl;
        return 2;
    }
}
