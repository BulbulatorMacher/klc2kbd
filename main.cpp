#include "codepage.h"

int main(int argc, char **argv)
{
    std::string cpFilename;
    for (int i = 0; i < argc; ++i) {
        if ((std::string(argv[i]) == "--cp") && ((i + 1) < argc)) {
            cpFilename = argv[i + 1];
        }
    }

    Codepage cp(cpFilename);

    return 0;
}
