#pragma once

#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

class ConfigItem {
public:
    ~ConfigItem();

    static ConfigItem* create(std::string path, int depth);

    int getDepth() const;

    std::string getPath();

private:
    int depth;
    std::string path;

    ConfigItem(std::string path, int depth);
};
