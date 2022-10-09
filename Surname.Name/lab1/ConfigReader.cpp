#include "ConfigReader.h"

// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

ConfigItem* ConfigItem::create(std::string path, int depth) {
    if (depth <= 1) {
        std::cout << "Depth should be greater then one" << std::endl;
    } else {
        trim(path);
        if (fs::exists(path)) {
            if (fs::is_directory(path)) {
                return new ConfigItem(path, depth);
            } else {
                std::cout << "Path should be directory" << std::endl;
            }
        } else {
            std::cout << "Path does not exist" << std::endl;
        }
    }
    return nullptr;
}


void ConfigReader::read(fs::path& config) {
    std::ifstream input(config);
    int size = 0;
    for (std::string line; getline(input, line);) {
        size++;
        std::string number = "";
        trim(line);
        int n = line.size();
        for (int i = n - 1; i >= 0; --i) {
            if (isdigit(line[i]))
                number = line[i] + number;
            else {
                try {
                    int depth = stoi(number);

                    auto path = line.substr(0, i);
                    ConfigItem* item = ConfigItem::create(path, depth);
                    if (item)
                        items->push_back(item);
                    else
                        this->isCorrect = false;
                    break;
                }
                catch (...) { "Wrong config"; }


            }
        }
    }
    std::cout << "Read " << items->size() << " correct lines of " << size << std::endl;
    if (size != items->size())
        this->isCorrect = false;
}

void ConfigReader::print() {
    for (auto item: *items) {
        std::cout << "Path: " << item->getPath() << " Depth: " << item->getDepth() << std::endl;
    }
}

void ConfigReader::clean() {
    isCorrect = true;
    for (auto item: *(this->items))
        delete item;
}

ConfigReader::ConfigReader() {
    items = new std::vector<ConfigItem*>();

}

ConfigReader::~ConfigReader() {
    clean();
    delete items;
}
