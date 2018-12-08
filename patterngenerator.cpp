#include "patterngenerator.h"
#include <redasm/support/utils.h>
#include <redasm/support/hash.h>
#include <fstream>
#include <json.hpp>

#define WILDCARD_PATTERN        "??"
#define PATTERN_OFFSET(offset)  (offset / 2)
#define PATTERN_SIZE(size)      PATTERN_OFFSET(size)
#define SIGNATURE_SIZE(pattern) PATTERN_SIZE(pattern.size())

using json = nlohmann::json;

PatternGenerator::PatternGenerator(): std::list<BytePattern>() { }

bool PatternGenerator::saveAsJSON(const std::string &jsonfile)
{
    std::fstream fs(jsonfile + ".json", std::ios::out | std::ios::trunc);

    if(!fs.is_open())
        return false;

    auto patterns = json::array();

    for(auto it = this->begin(); it != this->end(); it++)
    {
        auto patternobj = json::object();
        patternobj["pattern"] = it->pattern;
        patternobj["names"] = json::array();

        for(auto& name : it->names)
        {
            auto nameobj = json::object();
            nameobj["offset"] = name.offset;
            nameobj["name"] = name.name;
            nameobj["symboltype"] = name.symboltype;

            patternobj["names"].push_back(nameobj);
        }

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
        if(it->pattern.empty())
            continue;

        if(it->names.empty())
        {
            std::cout << "WARNING: Skipping anonymous pattern: " << REDasm::quoted(it->pattern) << std::endl;
            continue;
        }

        REDasm::Signature sig;
        sig.size = SIGNATURE_SIZE(it->pattern);

        this->setFirstAndLast(&sig, *it);

        if(!this->appendAllPatterns(&sig, *it))
            return false;

        this->appendAllNames(&sig, *it);
        sigdb << sig;
    }

    return sigdb.save(sdbfile + ".sdb");
}

void PatternGenerator::setFirstAndLast(REDasm::Signature *signature, const BytePattern& bytepattern) const
{
    signature->first.type = REDasm::SignaturePatternType::Byte;

    for(s64 i = 0; i < bytepattern.pattern.size(); i += 2)
    {
        if(bytepattern.pattern.substr(i, 2) == WILDCARD_PATTERN)
            continue;

        signature->first.offset = PATTERN_OFFSET(i);
        signature->first.byte = REDasm::byte(bytepattern.pattern, i);
        break;
    }

    signature->last.type = REDasm::SignaturePatternType::Byte;

    for(s64 i = bytepattern.pattern.size() - 2; i >= 0 ; i -= 2)
    {
        if(bytepattern.pattern.substr(i, 2) == WILDCARD_PATTERN)
            continue;

        signature->last.offset = PATTERN_OFFSET(i);
        signature->last.byte = REDasm::byte(bytepattern.pattern, i);
        break;
    }
}

void PatternGenerator::appendAllNames(REDasm::Signature *signature, const BytePattern &bytepattern) const
{
    for(auto it = bytepattern.names.begin(); it != bytepattern.names.end(); it++)
        signature->symbols.push_back({ it->name, it->offset, it->symboltype });
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
            sigpatt.checksum = this->chuckChecksum(chunk);
        }
        else
            sigpatt.type = REDasm::SignaturePatternType::Skip;

        offset += chunk.size();
        signature->patterns.push_back(sigpatt);
    }

    return true;
}

u16 PatternGenerator::chuckChecksum(const std::string &chunk) const
{
    std::vector<u8> bytes(SIGNATURE_SIZE(chunk));

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

void PatternGenerator::wildcard(BytePattern *bytepattern, size_t pos, size_t n)
{
    pos *= 2;

    if(pos >= bytepattern->pattern.size())
    {
        std::cout << "WARNING: " << pos << " >= " << bytepattern->pattern.size() << std::endl;
        return;
    }

    n *= 2;
    bytepattern->pattern.replace(pos, n, WILDCARD_PATTERN);
}

std::string PatternGenerator::fullname(const std::string &prefix, const std::string &name)
{
    return prefix.empty() ? name : (prefix + "." + name);
}
