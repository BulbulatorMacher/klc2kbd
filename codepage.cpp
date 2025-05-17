#include "codepage.h"

#include <fstream>
#include <sstream>

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
