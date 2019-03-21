#ifndef PATTERNGENERATOR_H
#define PATTERNGENERATOR_H

#define WILDCARD_PATTERN        "??"
#define WILDCARD_CHARACTER      '?'

#include <algorithm>
#include <string>
#include <list>
#include <redasm/disassembler/types/symboltable.h>
#include <redasm/disassembler/disassembler.h>
#include <redasm/database/signaturedb.h>
#include <json.hpp>

using json = nlohmann::json;

struct BytePattern
{
    u32 symboltype;
    std::string name, pattern;
};

class PatternGenerator: public std::list<BytePattern>
{
    public:
        PatternGenerator();
        void setPrefix(const std::string& prefix);
        void setSuffix(const std::string& suffix);
        bool disassemble(const BytePattern& bytepattern);
        bool writePatterns(REDasm::SignatureDB& sigdb);
        bool writePatternsSource(json& patterns);

    public:
        virtual std::string name() const = 0;
        virtual std::string assembler() const = 0;
        virtual bool test(const std::string& infile) = 0;
        virtual void generate(const std::string& infile) = 0;

    private:
        bool appendAllPatterns(REDasm::Signature* signature, const BytePattern &bytepattern) const;
        u16 chunkChecksum(const std::string& chunk) const;
        std::string getChunk(const std::string& s, int offset, bool *wildcard) const;
        std::string fullName(const std::string& name) const;
        REDasm::Disassembler *createDisassembler(REDasm::AbstractBuffer *buffer);

    protected:
        void pushPattern(u32 symboltype, const std::string& name, const std::string& pattern);
        bool isBytePatternValid(const BytePattern& bytepattern) const;

    protected:
        static std::string subPattern(const std::string& pattern, size_t pos, size_t len);
        static void wildcard(std::string& subPattern, size_t pos, size_t n);

    private:
        std::string m_outfolder, m_prefix, m_suffix;
};

#endif // PATTERNGENERATOR_H
