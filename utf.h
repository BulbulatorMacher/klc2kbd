#ifndef UTF_H
#define UTF_H

#include <string>

class Codepage;

namespace utf
{
    std::string toAscii(const std::string &filename, const Codepage &codepage);
};

#endif // UTF_H
