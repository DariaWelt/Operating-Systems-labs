#include <iostream>
#include "Daemon.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    // Check number of command line arguments
    if (argc != 2) {
        std::cout << "Wrong arguments count!";
        exit(EXIT_FAILURE);
    }
    Daemon::getInstance().setConfig(argv[1]);
    Daemon::getInstance().run();
    return EXIT_SUCCESS;
}
