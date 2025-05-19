/*
 * Copyright (c) 2025 BulbulatorMacher
 * Subject to GPL 2.0 license
 * Part of klc2kbd
 */

#ifndef UTF_H
#define UTF_H

#include <string>

class Codepage;

namespace utf
{
    std::string toAscii(const std::string &filename, const Codepage &codepage);
};

#endif // UTF_H
