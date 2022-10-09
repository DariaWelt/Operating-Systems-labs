#include "ConfigItem.h"

ConfigItem::~ConfigItem() = default;

int ConfigItem::getDepth() const {
    return depth;
}

std::string ConfigItem::getPath() {
    return path;
}

ConfigItem::ConfigItem(std::string path, int depth) {
    this->depth = depth;
    this->path = path;
}