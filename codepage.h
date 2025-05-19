/*
 * Copyright (c) 2025 BulbulatorMacher
 * Subject to GPL 2.0 license
 * Part of klc2kbd
 */

#ifndef CODEPAGE_H
#define CODEPAGE_H

#include <cstdint>
#include <string>
#include <unordered_map>

class Codepage
{
private:
    Codepage() = default;
public:
    Codepage(const std::string &filename);
    static Codepage Win1252();

    bool hasCode(uint8_t code) const;
    bool hasUnicode(uint16_t unicode) const;
    uint8_t code(uint16_t unicode) const;
    uint16_t unicode(uint8_t code) const;

private:
    std::unordered_map<uint8_t, uint16_t> m_codes;
};

#endif // CODEPAGE_H
