#include <fstream>
#include <sstream>
#include <iostream>
#include "ConfigParser.h"

ConfigParser ConfigParser::instance;

ConfigParser &ConfigParser::getInstance() {
    return instance;
}

int ConfigParser::parse(const std::string& filename) {
    std::ifstream config(filename.c_str());

    if (!config.is_open()){
        throw std::runtime_error("File cannot be opened");
    }
    params.clear();

    for (size_t i = 0; i < tokens.size(); i++) {
        std::string key;
        std::string value;

        try {
            config >> key;
            config >> value;

            params.insert({tokens.at(key), value});
        }
        catch (...) {
            throw std::runtime_error("Invalid config");
        }
    }

    config.close();

    return 0;
}
