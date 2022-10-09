#pragma once

#include "ConfigItem.h"

class ConfigReader;

class ConfigReader {

private:
    std::vector<ConfigItem*>* items = nullptr;
    bool isCorrect = true;
public:
    void read(fs::path& config);

    bool getIsCorrect() const { return isCorrect; };

    void print();

    void clean();

    std::vector<ConfigItem*>* getItems() { return items; };

    ConfigReader();

    ~ConfigReader();
};