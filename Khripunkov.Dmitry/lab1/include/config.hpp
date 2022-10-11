#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

class Config {
  public:
    Config() = default;
    Config(const std::string &filepath) { read(filepath); }

    void read(const std::string &filepath) {
        entries.clear();

        std::ifstream f(filepath);
        std::string dir1, dir2, time;

        while (f >> dir1 >> dir2 >> time) {
            entries.push_back(std::make_tuple(fs::path(dir1), fs::path(dir2), std::time_t(std::stoi(time))));
        }
    }

    inline const std::vector<std::tuple<fs::path, fs::path, std::time_t>> &get_entries() const { return entries; }

  private:
    std::vector<std::tuple<fs::path, fs::path, std::time_t>> entries;
};
