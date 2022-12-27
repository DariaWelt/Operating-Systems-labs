#ifndef LAB1_DAEMON_H
#define LAB1_DAEMON_H

#include "ConfigParser.h"
#include <memory>
#include <sys/syslog.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <csignal>
#include <fstream>
#include <filesystem>

class Daemon
{
public:
    static Daemon& getInstance();

    bool init(std::string const&configFileName);

    void run();

    void stop();

private:

    bool isWorking = false;

    int time;

    std::string dir1;

    std::string homePath;

    std::string fullConfigPath;

    std::string pidFilePath;

    Daemon() {};

    Daemon(Daemon const& daemon) = delete;

    static Daemon instance;

    std::string convertToPath(const std::string &configFileName);

    static void signalHandler(int signal);

    bool handlePidFile();

    bool setPidFile();

    bool setConfigParams();

    bool doTask();
};


#endif //LAB1_DAEMON_H
