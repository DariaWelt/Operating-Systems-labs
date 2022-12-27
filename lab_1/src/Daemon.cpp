#include <sys/syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <csignal>
#include "filesystem"
#include "Daemon.h"

constexpr char s_ident[] = "YUPICHKIN_DAEMON";
constexpr char s_pidFilePath[] = "/var/run/yupichkin_lab1.pid";

Daemon* Daemon::m_instance;

using namespace std;

RC Daemon::setPid()
{
  ifstream pidFile(s_pidFilePath);
  if(!pidFile)
  {
    syslog(LOG_ERR, "PID file can not be opened");
    return RC::FILE_CLOSED;
  }
  if(!pidFile.eof())
  {
    pid_t oldPid;
    pidFile >> oldPid;
    syslog(LOG_INFO, "Resetting PID %i", oldPid);
    if (filesystem::exists("/proc/" + std::to_string(oldPid))) {
      kill(oldPid, SIGTERM);
      pidFile.clear();
    }
  }
  pidFile.close();
  ofstream ofFile(s_pidFilePath);
  ofFile << getpid();
  syslog(LOG_INFO, "PID set %i", getpid());
  return RC::SUCCESS;
}

RC Daemon::makeFork()
{
  pid_t pid = fork();
  if (pid == -1)
  {
    syslog(LOG_ERR, "Fork failed");
    return RC::FAILED_FORK;
  }
  else if (pid)
  {
    return RC::PARENT_PROCESS;
  }

  pid = setsid();
  if (pid == -1)
  {
    syslog(LOG_ERR, "Failed creating session and setting the process group ID");
    return RC::FAILED_FORK;
  }

  pid = fork();
  if (pid == -1)
  {
    syslog(LOG_ERR, "Fork failed");
    return RC::FAILED_FORK;
  }
  else if (pid)
  {
    return RC::PARENT_PROCESS;
  }

  umask(0);

  if ((chdir("/")) == -1)
  {
    syslog(LOG_ERR, "ERROR: Failed in chdir: %d", errno);
    return RC::FAILED_CHANGE_DIR;
  }

  return setPid();
}

RC Daemon::loadConfig(const string &configFilePath)
{
  ifstream configFile = ifstream(configFilePath.c_str());
  if (!configFile) {
    syslog(LOG_ERR, "Config file can not be opened");
    return RC::FILE_DOES_NOT_EXIST;
  }
  RC rc;
  while (!configFile.eof()) {
    auto lineParams = ConfigParser::read_line(configFile, &rc, m_absolutePath);
    if (rc != RC::SUCCESS && rc != RC::EMPTY_STRING) {
      syslog(LOG_ERR, "Parsing config error %i", rc);
      return rc;
    }
    if (rc != RC::EMPTY_STRING) m_configParams.push_front(lineParams);
  }
  return RC::SUCCESS;
}

void Daemon::signalHandler(int signal)
{
  switch (signal) {
    case SIGTERM:
      syslog(LOG_INFO, "Daemon terminated");
      getInstance()->m_inited = false;
      break;
    case SIGHUP:
      getInstance()->m_configParams.clear();
      if(getInstance()->loadConfig(getInstance()->m_absolutePath + getInstance()->m_configPath))
      {
        syslog(LOG_ERR, "Loading config file failed");
        getInstance()->m_configParams.clear();
      }
      else
      {
        syslog(LOG_INFO, "Config reloaded");
      }
      break;
    default:
      syslog(LOG_ERR, "Unexpected signal");
  }
}

void Daemon::init(string const& configFilePath)
{
  m_inited = false;
  m_configPath = configFilePath;
  openlog(s_ident, LOG_NDELAY, LOG_USER);
  syslog(LOG_INFO, "Initialization started");

  m_absolutePath = filesystem::current_path().string() + "/";

  if(loadConfig(configFilePath) != RC::SUCCESS)
  {
    syslog(LOG_ERR, "Loading config file failed");
    m_configParams.clear();
    return;
  }
  syslog(LOG_INFO, "Loading config file succeed");

  if (makeFork() != RC::SUCCESS) return;

  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  syslog(LOG_INFO, "Initialization finished");
  m_inited = true;
}

int Daemon::execute()
{
  if(m_inited)
  {
    bool isValid = true;
    while (isValid && m_inited)
    {
      syslog(LOG_INFO, "Executed");
      for (const auto &paramSet: m_configParams) {
        isValid = cleanDir(paramSet.dstFolder);
        if(isValid)
          isValid = copyFilesFromDir(paramSet.srcFolder, paramSet.dstFolder, paramSet.extension, paramSet.subfolder);
        else
          break;
      }
      sleep(m_timeInterval);
    }
    return isValid;
  }
  syslog(LOG_ERR, "Is not inited");
  return 1;
}

bool Daemon::copyFilesFromDir(const string& srcDir, const string& dstDir,
                              const string& ext, const string& subfolder) {
  try
  {
    for (auto const &file: filesystem::directory_iterator(m_absolutePath + srcDir))
    {
      if (file.is_directory())
        continue;
      string dst = m_absolutePath + dstDir;
      dst += ext != filesystem::path(file).extension() ? "" : ("/" + subfolder);
      if(!filesystem::exists(dst))
      {
        filesystem::create_directory(dst);
      }

      filesystem::copy(file, dst + "/" + filesystem::path(file).filename().string());
    }
  }
  catch (const filesystem::filesystem_error& error)
  {
    syslog(LOG_ERR, "During copying files an error occurred %s", error.what());
    kill(getpid(), SIGTERM);
    return false;
  }
  return true;
}

bool Daemon::cleanDir(const std::string& path) {
  try
  {
    for (auto const &file: filesystem::directory_iterator(m_absolutePath +path))
    {
      if (file.is_directory())
        filesystem::remove_all(file);
      else
        filesystem::remove(file);
    }
  }
  catch (const filesystem::filesystem_error& error)
  {
    syslog(LOG_ERR, "During cleaning dir an error occurred %s", error.what());
    kill(getpid(), SIGTERM);
    return false;
  }
  return true;
}

Daemon *Daemon::getInstance()
{
  if (!m_instance)
    m_instance = new Daemon;
  return m_instance;
}
