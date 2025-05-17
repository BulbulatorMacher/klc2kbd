#include "klc.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

using namespace klc;
using namespace std;

CharDef klc::readUnicode(const string &str)
{
    if (str.empty()) {
        throw invalid_argument("unicode string is empty");
    }

    CharDef res;

    // dead key
    if (str.back() == '@') {
        res = readUnicode(str.substr(0, str.length() - 1));
        res.dead = true;
        return res;
    }

    // ligature
    if (str == "%%") {
        res.ligature = true;
        return res;
    }

    // printable ascii
    if (str.size() == 1) {
        const char c = str.front();
        if ((c >= '1' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            res.unicode = c;
            return res;
        }
    }

    // undefined
    if (str == "-1") {
        res.undefined = true;
        return res;
    }

    // unicode hex
    res.unicode = stoi(str, nullptr, 16);
    return res;
}

const vector<string> klc::VirtualKeys = {
    "","","","","","","","","","","","","","","","",  // 0x00 to 0x0F unused
    "","","","","","","","","","","","","","","","",  // 0x10 to 0x1F unused
    "SPACE",                                          // 0x20
    "","","","","","","","","","","","","","","",     // 0x21 to 0x2F unused
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", // 0x30 to 0x39
    "","","","","","","",                             // 0x3A to 0x40 unused
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", // 0x41 to 0x5A
    "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
    "U", "V", "W", "X", "Y", "Z",
    "","","","","",                                   // 0x5B to 0x5F unused
    "","","","","","","","","","","","","","",        // 0x60 to 0x6D unused
    "DECIMAL",                                        // 0x6E
    "",                                               // 0x6F         unused
    "","","","","","","","","","","","","","","","",  // 0x70 to 0x7F unused
    "","","","","","","","","","","","","","","","",  // 0x80 to 0x8F unused
    "","","","","","","","","","","","","","","","",  // 0x90 to 0x9F unused
    "","","","","","","","","","","","","","","","",  // 0xA0 to 0xAF unused
    "","","","","","","","","","",                    // 0xB0 to 0xB9 unused
    "OEM_1", "OEM_PLUS", "OEM_COMMA", "OEM_MINUS", "OEM_PERIOD", "OEM_2", "OEM_3", // 0xBA to 0xC0
    "","","","","","","","","","","","","","","",     // 0xC1 to 0xCF unused
    "","","","","","","","","","","",                 // 0xD0 to 0xDA unused
    "OEM_4", "OEM_5", "OEM_6", "OEM_7", "OEM_8",      // 0xDB to 0xDF
    "","",                                            // 0xE0 to 0xE1 unused
    "OEM_102",                                        // 0xE2
};

int klc::virtualKeyIdx(const string &virtualKey)
{
    if (virtualKey == "-1") {
        return -1;
    }
    auto it = find(VirtualKeys.begin(), VirtualKeys.end(), virtualKey);
    if (virtualKey == "" || it == VirtualKeys.end()) {
        throw runtime_error("could not find a virtual key");
    }
    return distance(VirtualKeys.begin(), it);
}

LayoutLine klc::readLayoutLine(const string &str)
{
    istringstream iss(str);
    string s;

    LayoutLine res;

    iss >> s;
    res.scanCode = stoi(s, nullptr, 16);

    iss >> s;
    res.virtualKey = virtualKeyIdx(s);

    iss >> s;
    if (s == "SGCap") {
        res.sgcap = true;
    } else {
        const uint8_t caps = stoi(s, nullptr, 16);
        res.capIsShift = caps & 0x01;
        res.capIsShiftAltGr = caps & 0x04;
    }

    while (true) {
        if (!(iss >> s) || s.empty() || s.find("//") == 0) {
            break;
        }
        res.charDefs.push_back(readUnicode(s));
    }

    return res;
}

const vector<string> klc::Sections = {
    "KBD", "COPYRIGHT", "COMPANY", "LOCALENAME", "LOCALEID", "VERSION",
    "ATTRIBUTES", "SHIFTSTATE", "LAYOUT", "LIGATURE", "DEADKEY",
    "KEYNAME", "KEYNAME_EXT", "KEYNAME_DEAD",
    "DESCRIPTIONS", "LANGUAGENAMES", "ENDKBD",
};
