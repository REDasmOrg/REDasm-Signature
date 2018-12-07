#ifndef PATTERNGENERATOR_H
#define PATTERNGENERATOR_H

#define WILDCARD_PATTERN "??"

#include <string>
#include <list>
#include <redasm/redasm_types.h>

typedef std::pair<offset_t, std::string> BytePatternNames;

struct BytePattern
{
    std::string pattern;
    std::list<BytePatternNames> names;
};

class PatternGenerator: public std::list<BytePattern>
{
    public:
        PatternGenerator();
        bool saveAsJSON(const std::string& jsonfile);
        bool saveAsSDB(const std::string& sdbfile);

    public:
        virtual std::string name() const = 0;
        virtual bool generate(const std::string& infile, const std::string& prefix = std::string()) = 0;

    protected:
        static void wildcard(BytePattern* bytepattern, size_t pos, size_t n);
        static std::string fullname(const std::string& prefix, const std::string& name);
};

#endif // PATTERNGENERATOR_H
