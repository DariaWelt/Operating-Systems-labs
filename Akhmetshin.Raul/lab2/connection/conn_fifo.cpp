#include "conn.h"

#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
    class ConnectionFifo : public Connection {
    private:
        static std::string const pathname;
         
        std::string name;

    public:
        ConnectionFifo(size_t id, bool create);
        ~ConnectionFifo();

        bool Read(void* buffer, size_t count) override;
        bool Write(void* buffer, size_t count) override;

    };

    std::string const ConnectionFifo::pathname = "/tmp/myfifo";
}

Connection* Connection::create(size_t id, bool create, size_t msg_size) {
    return new ConnectionFifo(id, create);
}

Connection::~Connection() {}


ConnectionFifo::ConnectionFifo(size_t id, bool create) {
    name = pathname + std::to_string(id);

    is_creater = create;

    if (create && mkfifo(name.c_str(), 0777) < 0) {
        std::cout << "Make fifo error" << std::endl;
    }


    desc = open(name.c_str(), O_RDWR | O_NONBLOCK);
    if (desc == -1) {
        std::cout << "Open error" << std::endl;
    }
}

ConnectionFifo::~ConnectionFifo() {
    close(desc);
    if (is_creater) {
        remove(name.c_str());
    }
}

bool ConnectionFifo::Read(void* buffer, size_t count) {
    bool res = true;

    int lenght = read(desc, buffer, count);
    if (lenght == -1) {
        res = false;
    }
    
    return res;
}

bool ConnectionFifo::Write(void* buffer, size_t count) {
    bool res = true;

    int length = write(desc, buffer, count);
    if (length == -1) {
        res = false;
    }

    return res;
}
