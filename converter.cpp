#include "converter.h"

#include "kbd.h"
#include "klc.h"
#include "utf.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Converter::Converter(const Codepage &codepage, const std::string &klcFilename)
    : codepage(codepage)
    , klcFilename(klcFilename)
{

}

std::vector<uint8_t> Converter::generateKbd() const
{
    kbd::Config kbd(codepage);

    const std::string fileData = utf::toAscii(klcFilename, codepage);
    std::istringstream file(fileData);

    std::string line;
    int lineNo = 0;
    std::string currentSection;

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
                } else if (section == "LOCALEID") {
                    std::string locIdQuot;
                    iss >> locIdQuot;
                    auto locIdHexStr = locIdQuot.substr(1, locIdQuot.size() - 1);
                    std::istringstream(locIdHexStr) >> std::hex >> kbd.localeId;
                } else if (section == "VERSION") {
                    // todo
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
                    kbd.ssCaps.addKey(l2.charDefs[ssNormIdx].unicode, l.virtualKey);
                }
                if (l2.charDefs.size() == 2 && !l2.charDefs[ssShiftIdx].undefined) {
                    kbd.ssCapsShift.addKey(l2.charDefs[ssShiftIdx].unicode, l.virtualKey);
                }
            }

            for (size_t i = 0; i < l.charDefs.size(); ++i) {
                const auto &cd = l.charDefs[i];
                if (cd.undefined) {
                    continue;
                }

                switch (shiftStates[i]) {
                case klc::ShiftState::NORM:
                    kbd.ssNormal.addKey(cd.unicode, l.virtualKey);
                    if (!l.sgcap) {
                        (l.capIsShift ? kbd.ssCapsShift : kbd.ssCaps).addKey(cd.unicode, l.virtualKey);
                    }
                    break;
                case klc::ShiftState::SHIFT:
                    kbd.ssShift.addKey(cd.unicode, l.virtualKey);
                    if (!l.sgcap) {
                        (l.capIsShift ? kbd.ssCaps : kbd.ssCapsShift).addKey(cd.unicode, l.virtualKey);
                    }
                    break;
                    continue;
                case klc::ShiftState::CTRL:
                    kbd.ssCtrl.addKey(cd.unicode, l.virtualKey);
                    break;
                case klc::ShiftState::SHIFT_CTRL:
                    kbd.ssCtrlShift.addKey(cd.unicode, l.virtualKey);
                    break;
                case klc::ShiftState::ALTGR:
                    kbd.ssAltGr.addKey(cd.unicode, l.virtualKey);
                    (l.capIsShiftAltGr ? kbd.ssCapsAltGrShift : kbd.ssCapsAltGr).addKey(cd.unicode, l.virtualKey);
                    break;
                case klc::ShiftState::SHIFT_ALTGR:
                    kbd.ssAltGrShift.addKey(cd.unicode, l.virtualKey);
                    (l.capIsShiftAltGr ? kbd.ssCapsAltGr : kbd.ssCapsAltGrShift).addKey(cd.unicode, l.virtualKey);
                    break;
                }
            }

            kbd.keys.push_back({(uint8_t)l.scanCode, (uint8_t)l.virtualKey, l.charDefs[l.capIsShift ? ssShiftIdx : ssNormIdx].unicode});
        }
    }

    return kbd.generateKbd();
}
