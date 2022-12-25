#include "goat.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Invalid arguments. Must be one args - host pid" << std::endl;
        return -1;
    }

    int host_pid = std::stoi(argv[1]);

    if (host_pid <= 0) {
        std::cout << "Invalid pid" << std::endl;
        return -1;
    }

    Goat& goat = Goat::instanse();
    if (!goat.connect_to_host(host_pid)) {
        std::cout << "Can not connect" << std::endl;
        return -1;
    }

    return goat.run();
}
