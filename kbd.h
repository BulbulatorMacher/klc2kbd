#ifndef KBD_H
#define KBD_H

#include <cstdint>
#include <string>
#include <vector>

class Codepage;

namespace kbd
{
    enum class ShiftState {
        NORM             = 0,
        ALT              = 1,
        SHIFT            = 2,
        ALT_SHIFT        = 3,
        CTRL             = 4,
        CTRL_SHIFT       = 6,
        ALTGR            = 5,
        ALTGR_SHIFT      = 7,
        CAPS             = 8,
        CAPS_ALT         = 9,
        CAPS_SHIFT       = 10,
        CAPS_ALT_SHIFT   = 11,
        CAPS_CTRL        = 12,
        CAPS_CTRL_SHIFT  = 14,
        CAPS_ALTGR       = 13,
        CAPS_ALTGR_SHIFT = 15

    };

    struct KeyDef {
        uint8_t scancode;
        uint8_t vkey;
        uint16_t ascii;
    };

    struct DeadKey {
        uint16_t   deadKey;
        uint8_t    vkey;
        ShiftState shiftState;
    };

    struct DeadKeyTrans {
        uint16_t deadKey;
        uint16_t key;
        uint16_t result;
    };

    class ShiftStateKeys {
    public:
        ShiftStateKeys(const Codepage &codepage);
        void addKey(uint16_t unicode, uint8_t vkey);
        std::vector<uint8_t> ascii() const;
        std::vector<uint8_t> vkeys() const;

    private:
        const Codepage &m_codepage;
        std::vector<uint8_t> m_ascii;
        std::vector<uint8_t> m_vkeys;
    };

    class Config {
    private:
        const Codepage &codepage;
    public:
        Config(const Codepage &codepage);

        uint32_t localeId = 0;
        uint16_t version = 0;
        bool shiftLock = false;

        std::vector<KeyDef> keys;

        // no Alt for now, hardcoded Caps, missing Ligature
        ShiftStateKeys ssNormal{ codepage };
        ShiftStateKeys ssShift{ codepage };
        ShiftStateKeys ssAltGr{ codepage };
        ShiftStateKeys ssAltGrShift{ codepage };
        ShiftStateKeys ssCaps{ codepage };
        ShiftStateKeys ssCapsShift{ codepage };
        ShiftStateKeys ssCapsAltGr{ codepage };
        ShiftStateKeys ssCapsAltGrShift{ codepage };
        ShiftStateKeys ssCtrl{ codepage };
        ShiftStateKeys ssCtrlShift{ codepage };

        std::vector<DeadKey> deadKeys;
        std::vector<DeadKeyTrans> deadKeyTrans;

        std::vector<uint8_t> generateKbd() const;
    };
};

#endif // KBD_H
