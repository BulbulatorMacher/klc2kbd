#ifndef KLC_H
#define KLC_H

#include <cstdint>
#include <string>
#include <vector>

namespace klc
{

enum class ShiftState {
    NORM        = 0,
    SHIFT       = 1,
    CTRL        = 2,
    SHIFT_CTRL  = 3,
    ALTGR       = 6,
    SHIFT_ALTGR = 7
};

struct CharDef
{
    bool undefined = true;
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
bool isSection(const std::string &keyword);

std::string trimmedLine(const std::string &line);
std::string stripQuotes(const std::string &text);
}

#endif // KLC_H
