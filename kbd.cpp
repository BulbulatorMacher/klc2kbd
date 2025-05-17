#include "kbd.h"

#include "codepage.h"

#include <algorithm>
#include <functional>
#include <stdexcept>

constexpr uint16_t ALTGRUSED = 0x08;
constexpr uint16_t CAPSNORMAL = 0x02;
constexpr uint16_t SHIFTLOCKUSED = 0x40;
constexpr uint16_t DEADCOMBOS = 0x20;

using namespace kbd;

static std::vector<uint8_t> genBYTE(uint8_t v)
{
    return {v};
}

static std::vector<uint8_t> genWORD(uint16_t v)
{
    return {uint8_t(v & 0xFF), uint8_t(v >> 8)};
}

static std::vector<uint8_t> genDWORD(uint32_t v)
{
    return {uint8_t(v & 0xFF), uint8_t((v >> 8) & 0xFF), uint8_t((v >> 16) & 0xFF), uint8_t((v >> 24) & 0xFF)};
}

template <typename T>
static std::vector<T> &operator << (std::vector<T> &a, const std::vector<T> &b)
{
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

Config::Config(const Codepage &codepage)
    : codepage(codepage)
{

}

std::vector<uint8_t> Config::generateKbd() const
{
    const std::vector<std::reference_wrapper<const ShiftStateKeys>> shiftStateKeys {
        ssKeys.at(ShiftState::NORM),       ssKeys.at(ShiftState::NORM),
        ssKeys.at(ShiftState::SHIFT),      ssKeys.at(ShiftState::SHIFT),
        ssKeys.at(ShiftState::CTRL),       ssKeys.at(ShiftState::CTRL_SHIFT),
        ssKeys.at(ShiftState::ALTGR),      ssKeys.at(ShiftState::ALTGR_SHIFT),
        ssKeys.at(ShiftState::CAPS),       ssKeys.at(ShiftState::CAPS),
        ssKeys.at(ShiftState::CAPS_SHIFT), ssKeys.at(ShiftState::CAPS_SHIFT),
        ssKeys.at(ShiftState::CTRL),       ssKeys.at(ShiftState::CTRL_SHIFT),
        ssKeys.at(ShiftState::CAPS_ALTGR), ssKeys.at(ShiftState::CAPS_ALTGR_SHIFT),
    };

    constexpr size_t HeaderBlockSize = 0x1C;
    constexpr size_t KbdDataHeaderSize = 0x26;

    std::vector<uint8_t> HeaderBlock;
    std::vector<uint8_t> KbdDataHeader;
    std::vector<uint8_t> KbdData;

    uint16_t flags = 0x00;
    flags |= ALTGRUSED;
    if (shiftLock)
        flags |= SHIFTLOCKUSED;
    flags |= DEADCOMBOS;
    KbdDataHeader << genWORD(flags);

    const int nStateKeys = (flags & SHIFTLOCKUSED) ? 3 : 4;
    KbdDataHeader << genWORD(nStateKeys) << genWORD(shiftStateKeys.size() - 1);
    KbdDataHeader << genWORD(deadKeys.size()) << genWORD(ligatures.size());

    // States
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    for (int i = 0; i < nStateKeys; ++i) {
        static const std::vector<uint8_t> sk{0x12, 0x80, 0x10, 0x80, 0x11, 0x80, 0x14, 0x01};
        KbdData << genBYTE(sk[2 * i]) << genBYTE(sk[2 * i + 1]);
    }

    // ToAscStates
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size() + 1);
    for (size_t i = 0; i < shiftStateKeys.size(); ++i) {
        static const std::vector<uint8_t> ss{0, 1, 2, 3, 4, 6, 5, 7, 8, 9, 10, 11, 12, 14, 13, 15};
        KbdData << genBYTE(ss[i]);
    }

    // ToAscStateTables, ToAscVkeyList, ToAscVKeyListLens
    std::vector<uint8_t> ToAscStateTables, ToAscVkeyList, ToAscVKeyListLens;
    for (const ShiftStateKeys &ss : shiftStateKeys) {
        ToAscStateTables << genWORD(KbdDataHeaderSize + KbdData.size());
        KbdData << ss.ascii();
        ToAscVkeyList << genWORD(KbdDataHeaderSize + KbdData.size());
        KbdData << ss.vkeys();
        ToAscVKeyListLens << genBYTE(ss.vkeys().size());
    }
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << ToAscStateTables;
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << ToAscVkeyList;
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << ToAscVKeyListLens;

    // VKShiftStates
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    for (size_t i = 0; i < shiftStateKeys.size(); ++i) {
        static const std::vector<uint8_t> ss{0, 0, 1, 1, 2, 3, 6, 7, 4, 4, 5, 5, 2, 3, 6, 7};
        KbdData << genBYTE(ss[i]);
    }

    // ScanToIdx, VKeyToIdx, SCANSIZE, VKeyToAnsi
    std::vector<uint8_t> ScanToIdx, VKeyToIdx, VKeyToAnsi;
    for (auto &key : keys) {
        ScanToIdx  << genBYTE(key.scancode);
        VKeyToIdx  << genBYTE(key.vkey);
        VKeyToAnsi << genBYTE(codepage.code(key.ascii));
    }
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << ScanToIdx;
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << VKeyToIdx;
    KbdDataHeader << genWORD(keys.size());
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << VKeyToAnsi;

    // DeadKeyTable
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    for (auto dk : deadKeys) {
        KbdData << genBYTE(codepage.code(dk.deadKey));
        if (flags & DEADCOMBOS) {
            KbdData << genBYTE(dk.vkey) << genBYTE((int)dk.shiftState) << genBYTE(0);
        }
    }
    // DeadTrans
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    KbdData << genWORD(deadKeyTrans.size());
    for (auto dkt : deadKeyTrans) {
        KbdData << genBYTE(codepage.code(dkt.deadKey)) << genBYTE(codepage.code(dkt.key));
    }
    for (auto dkt : deadKeyTrans) {
        KbdData << genBYTE(codepage.code(dkt.result));
    }

    // LigKeys
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());
    for (auto lig : ligatures) {
        if (lig.chars.size() < 2) {
            throw std::runtime_error("invalid ligature");
        }
        KbdData << genBYTE(codepage.code(lig.chars[0]))
                << genBYTE(lig.vkey)
                << genBYTE((int)lig.shiftState)
                << genBYTE(0) << genBYTE(lig.chars.size() - 1);
        for (size_t i = 1; i < lig.chars.size(); ++i) {
            KbdData << genBYTE(codepage.code(lig.chars[i])) << genBYTE(0);
        }
    }
    KbdData << genBYTE(0) << genBYTE(0);

    // CapsBits
    KbdDataHeader << genWORD(KbdDataHeaderSize + KbdData.size());

    HeaderBlock << genBYTE('D') << genBYTE('S');
    HeaderBlock << genDWORD(localeId) << genWORD(version);
    HeaderBlock << genDWORD(HeaderBlockSize) << genWORD(KbdDataHeader.size() + KbdData.size());
    HeaderBlock << genDWORD(0) << genWORD(0) << genDWORD(0) << genDWORD(0);

    if (HeaderBlock.size() != HeaderBlockSize) {
        throw std::runtime_error("invalid HeaderBlock size");
    }
    if (KbdDataHeader.size() != KbdDataHeaderSize) {
        throw std::runtime_error("invalid KbdDataHeader size");
    }

    return HeaderBlock << KbdDataHeader << KbdData;
}

ShiftStateKeys::ShiftStateKeys(const Codepage &codepage)
    : m_codepage(codepage)
{

}

void ShiftStateKeys::addKey(uint16_t unicode, uint8_t vkey)
{
    if (std::find(m_vkeys.begin(), m_vkeys.end(), vkey) != m_vkeys.end()) {
        throw std::runtime_error("duplicate key");
    }

    m_ascii.push_back(m_codepage.code(unicode));
    m_vkeys.push_back(vkey);
}

std::vector<uint8_t> ShiftStateKeys::ascii() const
{
    return m_ascii;
}

std::vector<uint8_t> ShiftStateKeys::vkeys() const
{
    return m_vkeys;
}
