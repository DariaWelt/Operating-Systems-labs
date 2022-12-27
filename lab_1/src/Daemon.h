#ifndef DAEMON_DAEMON_H
#define DAEMON_DAEMON_H
#include <forward_list>
#include <string>

#include "ConfigParser.h"

class Daemon {
 private:
  static constexpr u_short m_timeInterval = 60;
  static Daemon* m_instance;
  std::forward_list<ConfigParser::Params> m_configParams;
  std::string m_absolutePath;
  std::string m_configPath;
  bool m_inited = false;

 private:
  Daemon() = default;
  RC loadConfig(std::string const& configFilePath);
  static RC makeFork();
  static RC setPid();
  static void signalHandler(int signal);
  bool copyOldFiles(const std::string& srcDir, const std::string& dstDir);

 public:
  void init(std::string const& configFilePath);
  int execute();

  static Daemon* getInstance();

  Daemon& operator=(Daemon const& daemon) = delete;
  Daemon(Daemon const& daemon) = delete;
};

#endif  // DAEMON_DAEMON_H
