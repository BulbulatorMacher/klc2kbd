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
    std::vector<uint8_t> generateKbd() const;

private:
    const Codepage &codepage;
    const std::string klcFilename;
};

#endif // CONVERTER_H
