#include "codepage.h"

#include <fstream>
#include <sstream>
#include <vector>

Codepage::Codepage(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("could not open codepage file");
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        ++lineNo;
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        if (line.empty()) {
            continue;
        }

        unsigned int code;
        unsigned int unicode;
        std::istringstream iss(line);
        if (!(iss >> std::hex >> code)) {
            throw std::runtime_error("error reading codepage file line " + std::to_string(lineNo));
            if (code > 0xFF) {
                throw std::runtime_error("codepage data out of range line " + std::to_string(lineNo));
            }
        }
        if (!(iss >> std::hex >> unicode)) {
            // code can be undefinded, in such case it does not have a unicode value
            continue;
        } else if (unicode > 0xFFFF) {
            throw std::runtime_error("codepage data out of range line " + std::to_string(lineNo));
        }

        if (hasCode(code)) {
            throw std::runtime_error("duplicate codepage value line" + std::to_string(lineNo));
        }
        m_codes.insert({code, unicode});
    }
}

Codepage Codepage::Win1252()
{
    Codepage cp;
    for (int c = 0; c <= 0xFF; ++c) {
        cp.m_codes.insert({c, c});
    }
    static const std::vector<uint16_t> ctr {
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
        0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
    };
    for (size_t i = 0; i < ctr.size(); ++i) {
        cp.m_codes[i + 0x80] = ctr[i];
    }
    return cp;
}

bool Codepage::hasCode(uint8_t code) const
{
    return m_codes.find(code) != m_codes.end();
}

bool Codepage::hasUnicode(uint16_t unicode) const
{
    for (auto it : m_codes) {
        if (it.second == unicode) {
            return true;
        }
    }
    return false;
}

uint8_t Codepage::code(uint16_t unicode) const
{
    for (auto it : m_codes) {
        if (it.second == unicode) {
            return it.first;
        }
    }
    throw std::runtime_error("does not have a unicode");
}

uint16_t Codepage::unicode(uint8_t code) const
{
    if (!hasCode(code)) {
        throw std::runtime_error("does not have a code");
    }
    return m_codes.find(code)->second;
}
