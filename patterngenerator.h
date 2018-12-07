#ifndef PATTERNGENERATOR_H
#define PATTERNGENERATOR_H

#include <algorithm>
#include <string>
#include <list>
#include <redasm/disassembler/types/symboltable.h>
#include <redasm/database/signaturedb.h>

struct BytePatternNames
{
    BytePatternNames(): symboltype(REDasm::SymbolTypes::Data) { }

    std::string name;
    offset_t offset;
    u32 symboltype;
};

struct BytePattern
{
    std::string pattern;
    std::list<BytePatternNames> names;

    void name(const std::string& name, offset_t offset, u32 symboltype) {
        BytePatternNames bpn;
        bpn.name = name;
        bpn.offset = offset;
        bpn.symboltype = symboltype;

        names.push_back(bpn);
    }
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

    private:
        void setFirstAndLast(REDasm::Signature* signature, const BytePattern &bytepattern) const;
        void appendAllNames(REDasm::Signature* signature, const BytePattern &bytepattern) const;
        bool appendAllPatterns(REDasm::Signature* signature, const BytePattern &bytepattern) const;
        u16 chuckChecksum(const std::string& chunk) const;
        std::string getChunk(const std::string& s, int offset, bool *wildcard) const;

    protected:
        static void wildcard(BytePattern* bytepattern, size_t pos, size_t n);
        static std::string fullname(const std::string& prefix, const std::string& name);
};

#endif // PATTERNGENERATOR_H
