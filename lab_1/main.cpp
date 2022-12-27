#include <iostream>
#include "ConfigParser.h"
#include "Daemon.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Arguments expected: 2 and actual " + std::to_string(argc);
        return 1;
    }

    Daemon& daemon = Daemon::getInstance();

    try {
        if (daemon.init(argv[1]))
            daemon.run();
    }
    catch (std::runtime_error &error) {
        std::cerr << "Fatal error in deamon " << error.what();
    }
}