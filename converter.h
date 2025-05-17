#ifndef CONVERTER_H
#define CONVERTER_H

#include <cstdint>
#include <string>
#include <vector>

class Codepage;

class Converter
{
public:
    Converter(const Codepage &codepage, const std::string &klcFilename);
    std::vector<uint8_t> generateKbd();
    std::string name() const;
    std::string description() const;
    uint32_t localeId() const;

private:
    const Codepage &codepage;
    const std::string klcFilename;

    bool m_generated = false;
    std::string m_name;
    std::string m_description;
    uint32_t m_localeId;
};

#endif // CONVERTER_H
