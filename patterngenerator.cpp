#include "patterngenerator.h"
#include <redasm/support/utils.h>
#include <redasm/support/hash.h>
#include <redasm/plugins/plugins.h>
#include <redasm/formats/binary/binary.h>
#include <fstream>
#include <json.hpp>

#define PATTERN_OFFSET(offset)  (offset / 2)
#define PATTERN_SIZE(size)      PATTERN_OFFSET(size)
#define SIGNATURE_BYTES(pattern) PATTERN_SIZE(pattern.size())

using json = nlohmann::json;

PatternGenerator::PatternGenerator(): std::list<BytePattern>() { }
void PatternGenerator::setOutputFolder(const std::string &s) { m_outfolder = s; }

bool PatternGenerator::saveAsJSON(const std::string &jsonfile)
{
    std::fstream fs(outputFile(jsonfile + ".json"), std::ios::out | std::ios::trunc);

    if(!fs.is_open())
        return false;

    auto patterns = json::array();

    for(auto it = this->begin(); it != this->end(); it++)
    {
        if(!this->isBytePatternValid(*it))
            continue;

        auto patternobj = json::object();
        patternobj["pattern"] = it->pattern;
        patternobj["name"] = it->name;
        patternobj["symboltype"] = it->symboltype;

        patterns.push_back(patternobj);
    }

    fs << patterns.dump(2);
    return true;
}

bool PatternGenerator::saveAsSDB(const std::string &sdbfile)
{
    REDasm::SignatureDB sigdb;

    for(auto it = this->begin(); it != this->end(); it++)
    {
        if(!this->isBytePatternValid(*it))
            continue;

        REDasm::Signature sig;
        sig.size = SIGNATURE_BYTES(it->pattern);
        sig.symboltype = it->symboltype;
        sig.name = it->name;

        if(!this->appendAllPatterns(&sig, *it))
            return false;

        sigdb << sig;
    }

    return sigdb.save(outputFile(sdbfile + ".sdb"));
}

bool PatternGenerator::disassemble(const std::string &pattern)
{
    RE_UNUSED(pattern);
    std::cout << "Disassembler is not supported" << std::endl;
    return false;
}

std::string PatternGenerator::outputFile(const std::string &filename) const
{
    std::string outfile = filename;

    if(!m_outfolder.empty())
        outfile = m_outfolder + REDasm::Runtime::rntDirSeparator + outfile;

    return outfile;
}

bool PatternGenerator::appendAllPatterns(REDasm::Signature *signature, const BytePattern &bytepattern) const
{
    int offset = 0;

    while(offset < bytepattern.pattern.size())
    {
        REDasm::SignaturePattern sigpatt;

        bool wildcard = false;
        std::string chunk = this->getChunk(bytepattern.pattern, offset, &wildcard);

        if(chunk.empty())
        {
            std::cout << "ERROR: Empty chunk @ " << REDasm::hex(offset) << std::endl;
            return false;
        }

        sigpatt.size = PATTERN_SIZE(chunk.size());

        if(!wildcard)
        {
            sigpatt.type = REDasm::SignaturePatternType::CheckSum;
            sigpatt.checksum = this->chunkChecksum(chunk);
        }
        else
            sigpatt.type = REDasm::SignaturePatternType::Skip;

        offset += chunk.size();
        signature->patterns.push_back(sigpatt);
    }

    return true;
}

bool PatternGenerator::isBytePatternValid(const BytePattern &bytepattern) const
{
    if(bytepattern.pattern.empty())
        return false;

    if(bytepattern.name.empty())
    {
        std::cout << "WARNING: Skipping anonymous pattern" << std::endl;
        return false;
    }

    int wc = 0;

    for(char ch : bytepattern.pattern)
    {
        if(ch == WILDCARD_CHARACTER)
            wc++;
    }

    if((wc / static_cast<float>(bytepattern.pattern.size())) >= 0.50)
    {
        std::cout << "WARNING: Skipping unrealiable pattern: " << REDasm::quoted(bytepattern.name) << std::endl;
        return false;
    }

    return true;
}

u16 PatternGenerator::chunkChecksum(const std::string &chunk) const
{
    std::vector<u8> bytes(SIGNATURE_BYTES(chunk));

    for(size_t i = 0, j = 0; i < chunk.size(); i += 2, j++)
        bytes[j] = REDasm::byte(chunk, i);

    return REDasm::Hash::crc16(bytes.data(), bytes.size());
}

std::string PatternGenerator::getChunk(const std::string &s, int offset, bool* wildcard) const
{
    std::string chunk, hexbyte = s.substr(offset, 2);

    if(hexbyte == WILDCARD_PATTERN)
    {
        *wildcard = true;

        while((offset < s.size()) && (hexbyte == WILDCARD_PATTERN))
        {
            chunk += hexbyte;
            offset += 2;
            hexbyte = s.substr(offset, 2);
        }
    }
    else
    {
        *wildcard = false;

        while((offset < s.size()) && (hexbyte != WILDCARD_PATTERN))
        {
            chunk += hexbyte;
            offset += 2;
            hexbyte = s.substr(offset, 2);
        }
    }

    return chunk;
}

REDasm::Disassembler *PatternGenerator::createDisassembler(const char* assemblerid, u32 bits, REDasm::Buffer& buffer)
{
    if(buffer.empty())
    {
        std::cout << "ERROR: Pattern is empty" << std::endl;
        return NULL;
    }

    REDasm::AssemblerPlugin* assembler = REDasm::getAssembler(assemblerid);

    if(!assembler)
    {
        std::cout << "ERROR: Invalid Assembler: " << REDasm::quoted(assemblerid) << std::endl;
        return NULL;
    }

    address_t baseaddress = 1u << (bits - 1);
    REDasm::BinaryFormat* format = new REDasm::BinaryFormat(buffer);
    format->build(assembler->name(), bits, 0, baseaddress, baseaddress, REDasm::SegmentTypes::Code);
    return new REDasm::Disassembler(assembler, format); // Takes ownership
}

void PatternGenerator::pushPattern(const std::string &name, const std::string &pattern, u32 symboltype) { this->push_back({ name, pattern, symboltype }); }
std::string PatternGenerator::subPattern(const std::string &pattern, size_t pos, size_t len) { return pattern.substr(pos, len); }

void PatternGenerator::wildcard(std::string &pattern, size_t pos, size_t n)
{
    pos *= 2;

    if(pos >= pattern.size())
    {
        std::cout << "WARNING: " << pos << " >= " << pattern.size() << std::endl;
        return;
    }

    for(size_t i = 0; i < n; i++, pos += 2)
        pattern.replace(pos, 2, WILDCARD_PATTERN);
}

std::string PatternGenerator::fullname(const std::string &prefix, const std::string &name) { return prefix.empty() ? name : (prefix + "." + name); }
