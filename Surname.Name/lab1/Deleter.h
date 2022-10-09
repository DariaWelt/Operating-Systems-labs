#pragma once

#include "ConfigReader.h"

//#define ConfigItem Item
class Deleter {
public:
    static void clean(ConfigItem* item);

    static ConfigItem* data;
    static std::vector<fs::path>* list;

    static void traversalTree(fs::path& curPath, int curDepth);
};

