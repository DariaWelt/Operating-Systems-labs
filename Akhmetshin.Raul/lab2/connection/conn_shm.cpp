#include "conn.h"

#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
    class ConnectionShm : public Connection {
    private:
        static std::string const pathname;
        void *_bufptr = nullptr;
        std::string name;
        const size_t _size = 1024;

    public:
        ConnectionShm(size_t id, bool create);
        ~ConnectionShm();

        bool Read(void* buffer, size_t count) override;
        bool Write(void* buffer, size_t count) override;

    };

    std::string const ConnectionShm::pathname = "myshm";
}

Connection* Connection::create(size_t id, bool create, size_t msg_size) {
    return new ConnectionShm(id, create);
}

Connection::~Connection() {}


ConnectionShm::ConnectionShm(size_t id, bool create) {
    name = pathname + std::to_string(id);

    is_creater = create;

    int f = is_creater ? O_CREAT | O_RDWR : O_RDWR;

    desc = shm_open(name.c_str(), f, 0666);
    if (desc == -1)
        std::cout<<"connection error"<< std::endl;


    ftruncate(desc, _size);

    _bufptr = mmap(0, _size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, desc, 0);
    if (_bufptr == MAP_FAILED) {
        close(desc);
        if (is_creater)
            shm_unlink(name.c_str());
        std::cout<<"object mapping error"<<std::endl;
    }
}

ConnectionShm::~ConnectionShm() {
    munmap(_bufptr, _size);
    close(desc);
    if (is_creater)
        shm_unlink(name.c_str());
}

bool ConnectionShm::Read(void* buffer, size_t count) {
    if (count > _size)
        return false;
    memcpy(buffer, _bufptr, count);
    return true;
}

bool ConnectionShm::Write(void* buffer, size_t count) {
    if (count > _size)
        return false;
    memcpy(_bufptr, buffer, count);
    return true;
}
