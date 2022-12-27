#include "ConfigParser.h"

#include <sstream>
#include <filesystem>
#include <vector>
#include <cstring>


using namespace std;

vector<string> ConfigParser::split(const std::string& src) {
  size_t pos = 0;
  vector<string> result;
  auto str = src;
  while ((pos = str.find(delimiter)) != string::npos) {
    result.push_back(str.substr(0, pos));
    str.erase(0, pos + strlen(delimiter));
  }
  result.push_back(str);
  return result;
}

ConfigParser::Params ConfigParser::read_line(ifstream& fs, RC* rc, const string& relativePath) {
  RC innerRC;
  RC* pRC = rc != nullptr ? rc : &innerRC;
  if (!fs.is_open()) {
    *pRC = RC::FILE_CLOSED;
    return {};
  }

  string line;

  getline(fs, line);
  if (line.empty()) {
    *pRC = RC::EMPTY_STRING;
    return {};
  }
  auto tokens = split(line);

  if (tokens.size() != 2) {
    *pRC = RC::WRONG_CONFIG_FORMAT;
    return {};
  }

  Params result;
  result.srcFolder = tokens[0];
  result.dstFolder = tokens[1];

  if (!filesystem::is_directory(relativePath + result.srcFolder) || !filesystem::is_directory(relativePath + result.dstFolder)) {
    *pRC = RC::FILE_DOES_NOT_EXIST;
    return {};
  }

  *pRC = RC::SUCCESS;

  return result;
}
