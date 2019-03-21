#include "generators.h"
#include <iostream>

#define WRAP_TO_STRING(...)   #__VA_ARGS__
#define GENERATOR(generator)  WRAP_TO_STRING(../generators/generator##_generator.h)

#define REGISTER_GENERATOR(T) Generators::registered.push_back([&](const std::string& infile, const std::string& prefix, const std::string& suffix) -> PatternGenerator* { \
                                 return Generators::generateCallback<T##Generator>(infile, prefix, suffix); \
                              })

/* *** Generators *** */
#include GENERATOR(psyqlib)
#include GENERATOR(json)

std::list<GeneratorCallback> Generators::registered;
std::list< std::unique_ptr<PatternGenerator> > Generators::active;

void Generators::init()
{
    if(!Generators::registered.empty())
        return;

    REGISTER_GENERATOR(PsyQLib);
    REGISTER_GENERATOR(JSON);
}

PatternGenerator *Generators::getPattern(const std::string &infile, const std::string &prefix, const std::string &suffix, bool verbose)
{
    for(auto it = active.begin(); it != active.end(); it++)
    {
        if(!(*it)->test(infile))
            continue;

        if(verbose)
            std::cout << (*it)->name() << ": " << infile << std::endl;

        return it->get();
    }

    for(auto it = registered.begin(); it != registered.end(); it++)
    {
        PatternGenerator* patterngenerator = (*it)(infile, prefix, suffix);

        if(!patterngenerator)
            continue;

        if(verbose)
            std::cout << patterngenerator->name() << ": " << infile << std::endl;

        active.emplace_back(patterngenerator);
        return active.back().get();
    }

    return NULL;
}
