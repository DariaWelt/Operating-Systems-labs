#include "ConfigReader.h"
#include "Deleter.h"

int main() {
    // parent_path is needed, because cmake_build_debug directory is joined (why?)
    fs::path curPath = fs::current_path().parent_path();
    fs::path config = curPath / fs::path("config");
    ConfigReader configReader;
    if (!fs::exists(config) || fs::is_directory(config))
        std::cout << "Trouble with config" << std::endl;
    else {
        configReader.read(config);
        configReader.print();
        if (configReader.getIsCorrect()) {
            // TODO
            //   manage already deleted dirs (seems, it works...)
            for (auto item: *(configReader.getItems())) {
                Deleter::clean(item);
            }
        } else
            std::cout << "Config is not correct";
    }
}