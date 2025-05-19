/*
 * Copyright (c) 2025 BulbulatorMacher
 * Subject to GPL 2.0 license
 * Part of klc2kbd
 */

#include "utf.h"

#include "codepage.h"

#include <fstream>
#include <stdexcept>

enum Encoding {
    ASCII,
    UTF8_BOM,
    UTF16BE,
    UTF16BE_BOM,
    UTF16LE,
    UTF16LE_BOM
};

static Encoding detectEncoding(char c0, char c1)
{
    const uint8_t C0 = c0, C1 = c1;
    if (C0 == 0xEF && C1 == 0xBB)
        return UTF8_BOM;
    if (C0 == 0xFF && C1 == 0xFE)
        return UTF16LE_BOM;
    if (C0 == 0xFE && C1 == 0xFF)
        return UTF16BE_BOM;
    if (C0 != 0x00 && C1 == 0x00)
        return UTF16LE;
    if (C0 == 0x00 && C1 != 0x00)
        return UTF16BE;
    return ASCII;
}

std::string utf::toAscii(const std::string &filename, const Codepage &codepage)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("could not open a file");
    }

    char c0, c1;
    file.read(&c0, 1);
    file.read(&c1, 1);

    const Encoding encoding = detectEncoding(c0, c1);
    const bool utf16 = encoding == UTF16BE  || encoding == UTF16BE_BOM || encoding == UTF16LE || encoding == UTF16LE_BOM;
    const bool be    = encoding == UTF16BE  || encoding == UTF16BE_BOM;
    const bool bom   = encoding == UTF8_BOM || encoding == UTF16BE_BOM || encoding == UTF16LE_BOM;

    std::string res;

    if (utf16) {
        if (!bom) {
            const uint16_t unicode = (uint8_t)(be ? c1 : c0) | ((uint8_t)(be ? c0 : c1) << 8);
            res.push_back(codepage.code(unicode));
        }

        while (file.read(be ? &c1 : &c0, 1)) {
            file.read(be ? &c0 : &c1, 1);
            const uint16_t unicode = (uint8_t)c0 | ((uint8_t)c1 << 8);
            res.push_back(codepage.code(unicode));
        }
    } else {
        if (!bom) {
            res.push_back(c0);
            res.push_back(c1);
        } else {
            // BOM is 3 bytes in UTF8
            file.read(&c0, 1);
        }

        while (file.read(&c0, 1)) {
            res.push_back(c0);
        }
    }

    return res;
}
