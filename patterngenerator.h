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
    std::string name, pattern, assembler;
    u32 bits, symboltype;
};

class PatternGenerator: public std::list<BytePattern>
{
    public:
        PatternGenerator();
        bool disassemble(const BytePattern& bytepattern);
        bool saveAsSDB(REDasm::SignatureDB& sigdb);
        bool saveAsJSON(json& patterns);

    public:
        virtual std::string name() const = 0;
        virtual bool generate(const std::string& infile, const std::string& prefix = std::string()) = 0;

    private:
        bool appendAllPatterns(REDasm::Signature* signature, const BytePattern &bytepattern) const;
        u16 chunkChecksum(const std::string& chunk) const;
        std::string getChunk(const std::string& s, int offset, bool *wildcard) const;
        REDasm::Disassembler *createDisassembler(const std::string &assemblerid, u32 bits, REDasm::Buffer &buffer);

    protected:
        void pushPattern(const std::string& name, const std::string& pattern, const std::string& assembler, u32 bits, u32 symboltype);
        bool isBytePatternValid(const BytePattern& bytepattern) const;

    protected:
        static std::string subPattern(const std::string& pattern, size_t pos, size_t len);
        static void wildcard(std::string& subPattern, size_t pos, size_t n);
        static std::string fullname(const std::string& prefix, const std::string& name);

    private:
        std::string m_outfolder;
};

#endif // PATTERNGENERATOR_H
