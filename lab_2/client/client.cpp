#include "goat.h"

int main (int argc, char* argv[]) {

    openlog("GameClient", LOG_PID | LOG_PERROR, LOG_DAEMON);
    syslog(LOG_INFO, "Client's pid is: %d", getpid());
    std::cout << "Client's pid is: " << getpid() << std::endl;

    if (argc != 2) {
        syslog(LOG_ERR, "Expected cmd argument: PID");
        std::cout << "Expected cmd argument: PID" << std::endl;
        return -1;
    }

    int hostPid = atoi(argv[1]);

    syslog(LOG_INFO, "Client is working with host (pid): %d", hostPid);
    std::cout << "Client is working with host (pid): " << hostPid << std::endl;

    try {
        Goat& goat = Goat::getInstance();
        goat.setHostPid(hostPid);

        goat.prepareGame();
        goat.startGame();
    } catch (std::runtime_error &e) {
        syslog(LOG_ERR, "%s", e.what());
        std::cout << e.what() << std::endl;
        closelog();
        return 1;
    }
    closelog();
    return 0;
}
