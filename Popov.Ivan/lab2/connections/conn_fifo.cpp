#include "conn_fifo.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

ConnectionFifo::ConnectionFifo(pid_t pid, bool isHost) {
    this->isHost = isHost;
    fifoName = "/tmp/fifo_" + std::to_string(pid);
}

std::unique_ptr<ConnectionInterface> ConnectionInterface::Create(pid_t pid, bool isHost) {
    return std::make_unique<ConnectionFifo>(pid, isHost);
}

void ConnectionFifo::Open(size_t id, bool create) {
    if (isHost) {
        unlink(fifoName.c_str());
        if (mkfifo(fifoName.c_str(), 0666)) {
            throw std::runtime_error("Creation error");
        }
    }
    fileDesc = ::open(fifoName.c_str(), O_RDWR);
    if (fileDesc == -1) {
        if (isHost) {
           unlink(fifoName.c_str()); 
        }
        throw std::runtime_error("Open error");
    }
}

void ConnectionFifo::Read(void *buf, size_t count) {
    if (read(fileDesc, buf, count) < 0) {
        throw std::runtime_error("Read error");
    }
}

void ConnectionFifo::Write(void *buf, size_t count) {
    if (write(fileDesc, buf, count) < 0) {
        throw std::runtime_error("Write error");
    }  
}

void ConnectionFifo::Close(void) {
    close(fileDesc);
    if (isHost) {
        unlink(fifoName.c_str()); 
    }
}
