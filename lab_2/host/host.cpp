#include <syslog.h>
#include <unistd.h>
#include "server.h"


int main(int argc, char**argv)
{
    openlog("host_client", LOG_NOWAIT | LOG_PID, LOG_LOCAL1);

    if (argc != 2)
    {
        syslog(LOG_ERR, "Number of clients should be the second arg");
        closelog();
        return 1;
    }

    int n = std::atoi(argv[1]);
    if (n <= 0)
    {
        syslog(LOG_ERR, "Number of clients should be positive");
        closelog();
        return 1;
    }

    server *server = server::getInstance();
    syslog(LOG_INFO, "Host started with pid %d.", getpid());
    server->setNumOfClients(n);
    syslog(LOG_INFO, "Number of clients is %d", n);
    server->start();
    syslog(LOG_INFO, "host: stopped.");
    closelog();

    return 0;
}
