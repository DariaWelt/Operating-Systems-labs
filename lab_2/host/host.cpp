#include "wolf.h"

int main (int argc, char* argv[]) {

    openlog("GameHost", LOG_PID | LOG_PERROR, LOG_DAEMON);
    syslog(LOG_INFO, "Host's pid is: %d", getpid());
    std::cout << "Host's pid is: " << getpid() << std::endl;

    try {
        Wolf& wolf = Wolf::getInstance();
        wolf.setHostPid(getpid());

        wolf.prepareGame();
        wolf.startGame();
    } catch (std::runtime_error &e) {
        syslog(LOG_ERR, "%s", e.what());
        std::cout << e.what() << std::endl;
        closelog();
        return 1;
    }

    closelog();
    return 0;
}