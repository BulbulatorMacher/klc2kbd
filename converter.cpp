#include "converter.h"

#include "kbd.h"
#include "klc.h"
#include "utf.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

static kbd::ShiftState toKbdShiftState(klc::ShiftState ss)
{
    switch (ss) {
    case klc::ShiftState::NORM:
        return kbd::ShiftState::NORM;
    case klc::ShiftState::SHIFT:
        return kbd::ShiftState::SHIFT;
    case klc::ShiftState::CTRL:
        return kbd::ShiftState::CTRL;
    case klc::ShiftState::SHIFT_CTRL:
        return kbd::ShiftState::CTRL_SHIFT;
    case klc::ShiftState::ALTGR:
        return kbd::ShiftState::ALTGR;
    case klc::ShiftState::SHIFT_ALTGR:
        return kbd::ShiftState::ALTGR_SHIFT;
    default:
        throw std::runtime_error("unknown shift state");
    }
}

Converter::Converter(const Codepage &codepage, const std::string &klcFilename)
    : codepage(codepage)
    , klcFilename(klcFilename)
{

}

std::vector<uint8_t> Converter::generateKbd()
{
    m_generated = false;

    kbd::Config kbd(codepage);

    const std::string fileData = utf::toAscii(klcFilename, codepage);
    std::istringstream file(fileData);

    std::string line;
    int lineNo = 0;
    std::string currentSection;
    uint16_t currentDeadKey = 0x0000;

    std::vector<klc::ShiftState> shiftStates;
    int ssNormIdx = -1;
    int ssShiftIdx = -1;
    int ssCtrlIdx = -1;
    int ssShiftCtrlIdx = -1;
    int ssAltGrIdx = -1;
    int ssShiftAltGrIdx = -1;

    while (std::getline(file, line)) {
        ++lineNo;

        line = klc::trimmedLine(line);
        if (line.empty()) {
            continue;
        }

        {
            std::istringstream iss(line);
            std::string section;
            iss >> section;

            if (klc::isSection(section)) {
                currentSection = section;

                if (section == "ENDKBD") {
                    // stop parsing
                    break;
                } else if (section == "KBD") {
                    iss >> m_name >> m_description;
                    m_description = klc::stripQuotes(m_description);
                } else if (section == "LOCALEID") {
                    std::string locIdQuot;
                    iss >> locIdQuot;
                    std::istringstream(klc::stripQuotes(locIdQuot)) >> std::hex >> m_localeId;
                    kbd.localeId = m_localeId;
                } else if (section == "VERSION") {
                    // todo
                } else if (section == "DEADKEY") {
                    iss >> std::hex >> currentDeadKey;
                } else {
                    std::cout << "ignored section: " << section << std::endl;
                }

                // go to the next line
                continue;
            }
        }

        if (currentSection == "SHIFTSTATE") {
            shiftStates.push_back(static_cast<klc::ShiftState>(std::stoi(line)));
            switch (shiftStates.back()) {
            case klc::ShiftState::NORM:
                ssNormIdx = shiftStates.size() - 1;
                break;
            case klc::ShiftState::SHIFT:
                ssShiftIdx = shiftStates.size() - 1;
                break;
            case klc::ShiftState::CTRL:
                ssCtrlIdx = shiftStates.size() - 1;
                break;
            case klc::ShiftState::SHIFT_CTRL:
                ssShiftCtrlIdx = shiftStates.size() - 1;
                break;
            case klc::ShiftState::ALTGR:
                ssAltGrIdx = shiftStates.size() - 1;
                break;
            case klc::ShiftState::SHIFT_ALTGR:
                ssShiftAltGrIdx = shiftStates.size() - 1;
                break;
            }
        } else if (currentSection == "LAYOUT") {
            const auto l = klc::readLayoutLine(line);
            if (l.scanCode == -1) {
                throw std::runtime_error("invalid scancode, line: " + std::to_string(lineNo));
            }
            if (l.virtualKey == -1) {
                throw std::runtime_error("invalid virtual key, line: " + std::to_string(lineNo));
            }
            if (l.charDefs.size() > shiftStates.size()) {
                throw std::runtime_error("invalid shift states, line: " + std::to_string(lineNo));
            }

            if (l.sgcap) {
                if (!std::getline(file, line)) {
                    throw std::runtime_error("unexpected end of file");
                }
                ++lineNo;

                const auto l2 = klc::readLayoutLine(line);
                if (l2.scanCode != -1 || l2.virtualKey != -1
                        || l2.capIsShift || l2.capIsShiftAltGr || l2.sgcap
                        || l2.charDefs.empty() || l2.charDefs.size() > 2) {
                    throw std::runtime_error("invalid SGCap, line: " + std::to_string(lineNo));
                }

                if (!l2.charDefs[ssNormIdx].undefined) {
                    kbd.ssKeys.at(kbd::ShiftState::CAPS).addKey(l2.charDefs[ssNormIdx].unicode, l.virtualKey);
                    if (l2.charDefs[ssNormIdx].dead) {
                        throw std::runtime_error("invalid dead key shift state, line: " + std::to_string(lineNo));
                    }
                }
                if (l2.charDefs.size() == 2 && !l2.charDefs[ssShiftIdx].undefined) {
                    kbd.ssKeys.at(kbd::ShiftState::CAPS_SHIFT).addKey(l2.charDefs[ssShiftIdx].unicode, l.virtualKey);
                    if (l2.charDefs[ssShiftIdx].dead) {
                        throw std::runtime_error("invalid dead key shift state, line: " + std::to_string(lineNo));
                    }
                }
            }

            for (size_t i = 0; i < l.charDefs.size(); ++i) {
                const auto &cd = l.charDefs[i];
                if (cd.undefined) {
                    continue;
                }

                switch (shiftStates[i]) {
                case klc::ShiftState::NORM:
                    kbd.ssKeys.at(kbd::ShiftState::NORM).addKey(cd.unicode, l.virtualKey);
                    if (!l.sgcap) {
                        kbd.ssKeys
                                .at(l.capIsShift ? kbd::ShiftState::CAPS_SHIFT : kbd::ShiftState::CAPS)
                                .addKey(cd.unicode, l.virtualKey);
                    }
                    break;
                case klc::ShiftState::SHIFT:
                    kbd.ssKeys.at(kbd::ShiftState::SHIFT).addKey(cd.unicode, l.virtualKey);
                    if (!l.sgcap) {
                        kbd.ssKeys
                                .at(l.capIsShift ? kbd::ShiftState::CAPS : kbd::ShiftState::CAPS_SHIFT)
                                .addKey(cd.unicode, l.virtualKey);
                    }
                    break;
                    continue;
                case klc::ShiftState::CTRL:
                    kbd.ssKeys.at(kbd::ShiftState::CTRL).addKey(cd.unicode, l.virtualKey);
                    break;
                case klc::ShiftState::SHIFT_CTRL:
                    kbd.ssKeys.at(kbd::ShiftState::CTRL_SHIFT).addKey(cd.unicode, l.virtualKey);
                    break;
                case klc::ShiftState::ALTGR:
                    kbd.ssKeys.at(kbd::ShiftState::ALTGR).addKey(cd.unicode, l.virtualKey);
                    kbd.ssKeys
                            .at(l.capIsShiftAltGr ? kbd::ShiftState::CAPS_ALTGR_SHIFT : kbd::ShiftState::CAPS_ALTGR)
                            .addKey(cd.unicode, l.virtualKey);
                    break;
                case klc::ShiftState::SHIFT_ALTGR:
                    kbd.ssKeys.at(kbd::ShiftState::ALTGR_SHIFT).addKey(cd.unicode, l.virtualKey);
                    kbd.ssKeys
                            .at(l.capIsShiftAltGr ? kbd::ShiftState::CAPS_ALTGR : kbd::ShiftState::CAPS_ALTGR_SHIFT)
                            .addKey(cd.unicode, l.virtualKey);
                    break;
                }

                if (cd.dead) {
                    kbd::DeadKey dk;
                    dk.deadKey = cd.unicode;
                    dk.vkey = l.virtualKey;
                    switch (shiftStates[i]) {
                    case klc::ShiftState::NORM:
                    case klc::ShiftState::SHIFT:
                    case klc::ShiftState::ALTGR:
                    case klc::ShiftState::SHIFT_ALTGR:
                        dk.shiftState = toKbdShiftState(shiftStates[i]);
                        break;
                    default:
                        throw std::runtime_error("invalid dead key shift state, line: " + std::to_string(lineNo));
                    }
                    kbd.deadKeys.push_back(dk);
                }
            }

            kbd.keys.push_back({(uint8_t)l.scanCode, (uint8_t)l.virtualKey, l.charDefs[l.capIsShift ? ssShiftIdx : ssNormIdx].unicode});
        } else if (currentSection == "DEADKEY") {
            std::istringstream iss(line);
            kbd::DeadKeyTrans dkt;
            dkt.deadKey = currentDeadKey;
            iss >> std::hex >> dkt.key;
            iss >> std::hex >> dkt.result;
            kbd.deadKeyTrans.push_back(dkt);
        }
    }

    m_generated = true;
    return kbd.generateKbd();
}

std::string Converter::name() const
{
    if (!m_generated) {
        throw std::runtime_error("kbd not generated");
    }
    return m_name;
}

std::string Converter::description() const
{
    if (!m_generated) {
        throw std::runtime_error("kbd not generated");
    }
    return m_description;
}

uint32_t Converter::localeId() const
{
    if (!m_generated) {
        throw std::runtime_error("kbd not generated");
    }
    return m_localeId;
}
