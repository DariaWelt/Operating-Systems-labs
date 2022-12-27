#ifndef OS_LABS_SPBPU_2022_PARSER_H
#define OS_LABS_SPBPU_2022_PARSER_H


#include <string>
#include <map>
#include <memory>

class ConfigParser {
public:
    enum ConfigParams
    {
        TIME,
        DIR1
    };

    static ConfigParser& getInstance();

    int parse(const std::string& filename);

    std::map<ConfigParams, std::string> getParams() {return params;}

private:
    ConfigParser() {};

    ConfigParser(ConfigParser const& configParser) = delete;

    static ConfigParser instance;

    std::map<ConfigParams, std::string> params;
    const std::map<std::string , ConfigParams> tokens =
            {{"TIME", ConfigParams::TIME},
             {"DIR1", ConfigParams::DIR1}};
};


#endif //OS_LABS_SPBPU_2021_PARSER_H
