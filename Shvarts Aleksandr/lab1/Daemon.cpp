#include "Daemon.h"
Daemon::Daemon() {
    openlog("daemonlog",  LOG_PID, LOG_LOCAL0);
    int pid = 0;
    auto isPidRegistred = [&]() {
        std::ifstream pidFile(PID_PATH);
        if (pidFile.is_open()) {
            if (pidFile >> pid && kill(pid, 0) == 0) {
                pidFile.close();
                return true;
            }
            pidFile.close();
        }
        return false;
    };
    if (isPidRegistred()) {
        syslog(LOG_INFO, "Kill old process");
        kill(pid, SIGTERM);
    }

    syslog(LOG_INFO, "Init and register daemon");
    daemonise();
    syslog(LOG_INFO, "Handle signals");
    handleSignals();
}

void Daemon::daemonise()
{
    pid_t pid, sid;
   // FILE* pid_fp;

    syslog(LOG_INFO, "Start daemonisation");

    //first fork
    pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR, "Error in the first fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        // parent
        exit(EXIT_SUCCESS);
    }

    //child
    sid = setsid();
    if (sid < 0)
    {
        syslog(LOG_ERR, "Error in making a new session");
        exit(EXIT_FAILURE);
    }

    //second fork
    pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR, "Error in the second fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        // parent
        exit(EXIT_SUCCESS);
    }

    pid = getpid();

    //Change working directory to Home directory
    syslog(LOG_INFO, "Change directory to '/'");
    if (chdir("/") == -1)
    {
        syslog(LOG_ERR, "Failed to change working directory");
        exit(EXIT_FAILURE);
    }

    umask(0);

    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
        close(x);
    int dv = open("/dev/null", O_RDWR);
    if (dv == -1) {
        syslog(LOG_CRIT, "Could not open /dev/null");
        exit(EXIT_FAILURE);
    }
    for (auto fileno: {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO})
        dup2(dv, fileno);
    syslog(LOG_NOTICE, "Redirected streams");

    //registration
    std::ofstream pidFile(PID_PATH, std::ios_base::out);

    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Error with opening pid file");
        exit(EXIT_FAILURE);
    }

    pidFile << getpid();
    pidFile.close();
    syslog(LOG_INFO, "Daemonization ends");
}

void Daemon::handleSignals()
{
    static auto manageSignals = [](int signal) {
        switch (signal) {
        case SIGHUP:
            Daemon::getInstance().setConfig("");
            break;
        case SIGTERM:
            Daemon::getInstance().terminate();
            break;
        default:
            break;
        }
    };
    std::signal(SIGHUP, manageSignals);
    std::signal(SIGTERM, manageSignals);
}

void Daemon::setConfig(const std::string& path = "")
{
    if (!path.empty()) {
        configPath = path;
        syslog(LOG_INFO, "Set config %s\n",path.c_str());
        isInitialised = true;
    }
    else {
        isInitialised = false;
        syslog(LOG_INFO, "Config reloaded\n");
    }
}

void Daemon::run()
{
    Notify notifier;
    notifier.loadConfig(configPath);
    //closelog();
    //openlog("Disk monitoring", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Starting disk monitor");

    while (!wasTerminated)
    {
        //if was SIGHUP
        if (!isInitialised) {
            notifier.loadConfig(configPath);
            isInitialised = true;
        }
        notifier.writeChanges();
    }
    syslog(LOG_INFO, "Daemon successfully finished\n");
    closelog();
}

void Daemon::terminate()
{
    wasTerminated = true;
    syslog(LOG_INFO, "Process terminated.");
    closelog();
}
