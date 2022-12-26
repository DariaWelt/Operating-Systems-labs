#include <set>
#include <string>
#include <syslog.h>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

#include <filesystem>
#include <sys/types.h>
#include <csignal>
#include <sys/inotify.h>
#include <climits>
#include <fcntl.h>
constexpr size_t EVENT_SIZE  = ( sizeof (struct inotify_event) );
constexpr size_t EVENT_BUF_LEN = ( 1024 * ( EVENT_SIZE + NAME_MAX+1) );
namespace fs = std::filesystem;
class Notify {
private:
	int fd;
	std::set<std::string> files;
    fd_set watch_set;
public:
	Notify();
	~Notify();
	void writeChanges();
	void loadConfig(const std::string& path);
};