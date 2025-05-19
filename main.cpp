/*
 * Copyright (c) 2025 BulbulatorMacher
 * Subject to GPL 2.0 license
 * Part of klc2kbd
 */

#include "codepage.h"
#include "converter.h"

#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
    std::cout << "Copyright (c) 2025 BulbulatorMacher" << std::endl;
    std::cout << "Subject to GPL 2.0 license" << std::endl;
    std::cout << "Part of klc2kbd" << std::endl << std::endl;

    std::string cpFilename;
    for (int i = 0; i < argc; ++i) {
        if ((std::string(argv[i]) == "--cp") && ((i + 1) < argc)) {
            cpFilename = argv[i + 1];
        }
    }
    if (cpFilename.empty()) {
        std::cout << "codepage file not provided, using Windows-1252" << std::endl;
    }

    std::string klcFilename;
    klcFilename = argv[argc - 1];

    std::string kbdFilename;
    for (int i = 0; i < argc; ++i) {
        if ((std::string(argv[i]) == "-o") && ((i + 1) < argc)) {
            kbdFilename = argv[i + 1];
        }
    }
    if (kbdFilename.empty()) {
        // use the same filename as klc, but change extension to kbd
        kbdFilename = klcFilename;
        kbdFilename[kbdFilename.size() - 2] = 'b';
        kbdFilename[kbdFilename.size() - 1] = 'd';
        std::cout << "output file name not provided, using " << kbdFilename << std::endl;
    }

    const Codepage cp = cpFilename.empty() ? Codepage::Win1252() : Codepage(cpFilename);
    Converter conv{cp, klcFilename};

    auto kbd = conv.generateKbd();

    std::ofstream kbdFile(kbdFilename, std::ios::binary);
    if (!kbdFile.is_open()) {
        throw std::runtime_error("could not open output file for writing");
    }
    kbdFile.write((char*)kbd.data(), kbd.size());

    std::cout << "SUCCESS" << std::endl;

    return 0;
}
