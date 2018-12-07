#include <iostream>
#include <redasm/database/signaturedb.h>

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

    if(argc >= 2)
    {
    }
    else
        showUsage();

    return 0;
}
