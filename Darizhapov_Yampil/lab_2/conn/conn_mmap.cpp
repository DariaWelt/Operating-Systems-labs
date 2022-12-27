#include "conn.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>

class ConnMmap : public Conn
{
public:
    ~ConnMmap()
    {
        if (munmap(mem, memSize) == -1)
        {
            std::cout << "Couldn't free shared memory" << std::endl;
            std::exit(1);
        }
    }

    void read(Message &msg) override { std::memcpy(&msg, mem, sizeof(msg)); }
    void write(const Message &msg) override { std::memcpy(mem, &msg, sizeof(msg)); }

private:
    ConnMmap(void* ptr, size_t size)
    {
        mem = ptr;
        memSize = size;
    }

    friend Conn* Conn::create(bool isHost);

    void *mem;
    size_t memSize;
};

Conn* Conn::create(bool isHost)
{
    static void* mem;
    const size_t defaultMemSize = 1024;

    if (isHost)
    {
        mem = mmap(NULL, defaultMemSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        if (mem == MAP_FAILED)
            throw std::runtime_error("Couldn't allocate shared memory with mmap");
    }
    return new ConnMmap(mem, defaultMemSize);
}