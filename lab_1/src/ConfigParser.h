#ifndef DAEMON_CONFIGPARSER_H
#define DAEMON_CONFIGPARSER_H


#include <string>
#include <array>
#include <fstream>
#include <vector>

#include "RC.h"

class ConfigParser
{
private:
  static constexpr char delimiter[] = ";";
private:
  static std::vector<std::string> split(const std::string& src);
public:
  struct Params
  {
    std::string srcFolder;
    std::string dstFolder;
    std::string extension;
    std::string subfolder;
  };
public:
  static Params read_line(std::ifstream& fs, RC* rc = nullptr, const std::string& relativePath = "");
};


#endif //DAEMON_CONFIGPARSER_H
