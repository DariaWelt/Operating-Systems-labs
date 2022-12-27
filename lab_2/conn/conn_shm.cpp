#include "conn.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

class ConnShm : public Conn
{
public:
    ~ConnShm()
    {
        if (munmap(mem, memSize) == -1 || shm_unlink(shmName.c_str()) == -1)
        {
            std::cout << "Couldn't free shared memory" << std::endl;
            std::exit(1);
        }
    }

    void read(Message &msg) override { std::memcpy(&msg, mem, sizeof(msg)); }
    void write(const Message &msg) override { std::memcpy(mem, &msg, sizeof(msg)); }

private:
    ConnShm(bool isHost)
    {
        const int fd = shm_open(shmName.c_str(), O_RDWR | O_CREAT, 0777);
        if (fd == -1)
            throw std::runtime_error("Couldn't open shared memory file descriptor");

        if (isHost)
            if (ftruncate(fd, memSize) == -1)
            throw std::runtime_error("Couldn't truncate file descriptor to given size");

        mem = mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED)
            throw std::runtime_error("Couldn't allocate shared memory with mmap");
    }

    friend Conn *Conn::create(bool isHost);

    const std::string shmName = "/my_shm";
    void* mem;
    const size_t memSize = 1024;
};

Conn* Conn::create(bool isHost) { return new ConnShm(isHost); }