#include "Deleter.h"
#include "ConfigItem.h"

ConfigItem* Deleter::data = nullptr;
std::vector<fs::path>* Deleter::list = nullptr;

void Deleter::clean(ConfigItem* item) {
    Deleter::data = item;
    fs::path root = fs::path(item->getPath());
    if (fs::exists(root)) {
        Deleter::list = new std::vector<fs::path>;
        traversalTree(root, 1);
        std::cout << "Delete dirs and files on level 2" << std::endl;
        for (auto& path: *list) {
            std::cout << path.filename() << std::endl;
            fs::remove_all(path);
        }
        delete list;
        list = nullptr;
    }
}

void Deleter::traversalTree(fs::path& curPath, int curDepth) {
    if (curDepth > Deleter::data->getDepth()) {
        for (auto& entry: fs::directory_iterator(curPath)) {
            fs::path root = fs::path(data->getPath());
            fs::path name = root / entry.path().filename();
            fs::rename(entry.path(), name);
        }
        return;
    }
    if (curDepth == 1) {
        for (auto& entry: fs::directory_iterator(curPath)) {
            Deleter::list->push_back(entry.path());
        }
    }
    for (auto& entry: fs::directory_iterator(curPath)) {
        if (entry.is_directory()) {
            Deleter::traversalTree(const_cast<std::filesystem::path&>(entry.path()), curDepth + 1);
        }
    }
}
