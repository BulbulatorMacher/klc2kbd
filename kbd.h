#ifndef KBD_H
#define KBD_H

#include <cstdint>
#include <string>
#include <vector>

class Codepage;

namespace kbd
{
    struct KeyDef {
        uint8_t scancode;
        uint8_t vkey;
        uint16_t ascii;
    };

    struct DeadKey {
        enum class ShiftState {
            NORM = 0,
            SHIFT = 2,
            ALTGR = 5,
            ALTGRSHIFT = 7
        };

        uint16_t   deadKey;
        uint8_t    vkey;
        ShiftState shiftState;
    };

    struct DeadKeyTrans {
        uint16_t deadKey;
        uint16_t key;
        uint16_t result;
    };

    class ShiftState {
    public:
        ShiftState(const Codepage &codepage);
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
        ShiftState ssNormal{ codepage };
        ShiftState ssShift{ codepage };
        ShiftState ssAltGr{ codepage };
        ShiftState ssAltGrShift{ codepage };
        ShiftState ssCaps{ codepage };
        ShiftState ssCapsShift{ codepage };
        ShiftState ssCapsAltGr{ codepage };
        ShiftState ssCapsAltGrShift{ codepage };
        ShiftState ssCtrl{ codepage };
        ShiftState ssCtrlShift{ codepage };

        std::vector<DeadKey> deadKeys;
        std::vector<DeadKeyTrans> deadKeyTrans;

        std::vector<uint8_t> generateKbd() const;
    };
};

#endif // KBD_H
