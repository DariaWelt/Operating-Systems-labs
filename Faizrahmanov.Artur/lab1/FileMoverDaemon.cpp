#include "FileMoverDaemon.h"
#include "Config.h"

#include <cstdlib>
#include <csignal>
#include <fstream>
#include <filesystem>
#include <fcntl.h>

FileMoverDaemon& FileMoverDaemon::getInstance()
{
    static FileMoverDaemon instance;
    return instance;
}

void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
        FileMoverDaemon::getInstance().stop();
        closelog();
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Read config");
        if (!Config::getInstance().readConfig())
            syslog(LOG_INFO, "Config is not in the correct format");
        break;
    default:
        syslog(LOG_INFO, "Unknown signal found");
        break;
    }
}

void FileMoverDaemon::initialize(const std::string &configPath)
{
    openlog("FileMoverDaemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Read config");
    Config::getInstance().setConfigPath(configPath);
    if (!Config::getInstance().readConfig())
        syslog(LOG_INFO, "Config is not in the correct format");

    destructOldPid();
    createPid();

    isRunning = true;
}

void FileMoverDaemon::createPid()
{
    syslog(LOG_INFO, "Forking");

    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    if ((chdir("/")) < 0)
    {
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Write new pid");
    std::ofstream file(PID_PATH);
    if (!file.is_open())
    {
        syslog(LOG_ERR, "Can't open pid file");
        exit(EXIT_FAILURE);
    }
    file << getpid();
    file.close();

    signal(SIGHUP, signalHandler);
    signal(SIGTERM, signalHandler);

    dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
    dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
    dup2(open("/dev/null", O_WRONLY), STDERR_FILENO);
}

void FileMoverDaemon::destructOldPid()
{
    syslog(LOG_INFO, "Destruct old pid if needed");
    std::ifstream file;

    file.open(PID_PATH);

    if (!file)
    {
        syslog(LOG_INFO, "Can't check that pid already exists");
        return;
    }

    int pid;
    if (file >> pid)
    {
        if (kill(pid, 0) == 0)
            kill(pid, SIGTERM);
    }

    file.close();
}

void FileMoverDaemon::run()
{
    while (isRunning)
    {
        if (Config::getInstance().isConfigReaded())
        {
            moveFiles();
            sleep(Config::getInstance().getSleepDuration());
        }
        else
        {
            syslog(LOG_INFO, "Config not read");
        }
    }
}

void FileMoverDaemon::moveFiles()
{
    syslog(LOG_INFO, "Start move files");

    do
    {
        std::string fromPath = Config::getInstance().getFromPath();
        std::string toPath = Config::getInstance().getToPath();
        std::string ext = Config::getInstance().getFileExt();

        if (pathExist(fromPath) && pathExist(toPath))
        {
            syslog(LOG_INFO, "Move files with %s extension from %s to %s", ext.c_str(), fromPath.c_str(), toPath.c_str());
            removeFiles(toPath);
            try
            {
                for (const auto &entry : std::filesystem::directory_iterator(fromPath))
                {
                    std::string file = entry.path().string();
                    if (file.substr(file.find_last_of('.') + 1) == ext)
                    {
                        std::filesystem::copy(file, toPath);
                        syslog(LOG_INFO, "%s moved", file.c_str());
                    }
                }
            }
            catch (const std::exception &e)
            {
                syslog(LOG_ERR, "%s", e.what());
            }
        }

    } while (Config::getInstance().next());
}

void FileMoverDaemon::removeFiles(const std::string &path)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::remove_all(entry.path());
    }
}

bool FileMoverDaemon::pathExist(const std::string &path) const
{
    bool exists = std::filesystem::exists(path);

    if (!exists)
    {
        syslog(LOG_INFO, "Folder not found %s", path.c_str());
    }

    return exists;
}

void FileMoverDaemon::stop()
{
    isRunning = false;
}