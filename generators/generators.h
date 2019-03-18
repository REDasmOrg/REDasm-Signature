#ifndef GENERATORS_H
#define GENERATORS_H

#include <functional>
#include <memory>
#include <string>
#include <list>
#include <redasm/database/signaturedb.h>
#include "../patterngenerator.h"

typedef std::function<PatternGenerator*(const std::string&, const std::string&, const std::string&)> GeneratorCallback;

struct Generators
{
    Generators() = delete;
    Generators(const Generators&) = delete;

    template<typename T> static PatternGenerator* generateCallback(const std::string& infile, const std::string& prefix, const std::string& suffix);
    static PatternGenerator* getPattern(const std::string& infile, const std::string& prefix, const std::string& suffix, bool verbose = true);
    static bool saveAsJSON(REDasm::SignatureDB& sigdb);
    static bool saveAsJSONSource(json& patterns);
    static void init();

    static std::list< std::unique_ptr<PatternGenerator> > active;
    static std::list<GeneratorCallback> registered;
};

template<typename T> PatternGenerator* Generators::generateCallback(const std::string& infile, const std::string& prefix, const std::string& suffix)
{
    std::unique_ptr<T> p = std::make_unique<T>();
    p->setPrefix(prefix);
    p->setSuffix(suffix);

    if(p->generate(infile))
        return p.release();

    return NULL;
}

#endif // GENERATORS_H
