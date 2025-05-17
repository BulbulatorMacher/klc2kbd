#ifndef KLC_H
#define KLC_H

#include <cstdint>
#include <string>
#include <vector>

namespace klc
{
struct CharDef
{
    bool undefined = false;
    uint16_t unicode = 0;
    bool dead = false;
    bool ligature = false;
};
CharDef readUnicode(const std::string &str);

extern const std::vector<std::string> VirtualKeys;
int virtualKeyIdx(const std::string &virtualKey);

struct LayoutLine
{
    int scanCode = -1;
    int virtualKey = -1;
    bool capIsShift = false;
    bool capIsShiftAltGr = false;
    bool sgcap = false;
    std::vector<CharDef> charDefs;
};
LayoutLine readLayoutLine(const std::string &str);

extern const std::vector<std::string> Sections;
}

#endif // KLC_H
