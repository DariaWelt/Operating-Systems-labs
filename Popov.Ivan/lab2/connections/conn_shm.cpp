#include "conn_shm.h"
#include "../messageQueue/messageQueue.h"
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

ConnectionShm::ConnectionShm(pid_t pid, bool isHost) {
    this->isHost = isHost;
    shmName = "shm_" + std::to_string(pid);
}

std::unique_ptr<ConnectionInterface> ConnectionInterface::Create(pid_t pid, bool isHost) {
    return std::make_unique<ConnectionShm>(pid, isHost);
}

void ConnectionShm::Open(size_t id, bool create) {
    if (isHost) {
        fileDesc = shm_open(shmName.c_str(),  O_CREAT | O_RDWR | O_EXCL, 0666);
    } else {
        fileDesc = shm_open(shmName.c_str(), O_RDWR, 0666);
    }
    if (fileDesc == -1) {
        throw std::runtime_error("Creation error");
    }

    ftruncate(fileDesc, shm_size);
    shmPtr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fileDesc, 0);
    if (shmPtr == MAP_FAILED) {
        close(fileDesc);
        if (isHost) {
            shm_unlink(shmName.c_str());
        }
        throw std::runtime_error("Open error");
    }
}

void ConnectionShm::Read(void *buf, size_t count) {
    if (count + shift > shm_size) {
        throw std::runtime_error("Reading error");
    }
    memcpy(buf, ((char *)shmPtr + shift), count);
    shift += count;
}

void ConnectionShm::Write(void *buf, size_t count) {
    if (count + shift > shm_size) {
        throw std::runtime_error("Writing error");
    }
    memcpy(((char *)shmPtr + shift), buf, count);
    shift += count;
}

void ConnectionShm::Close(void) {
    munmap(shmPtr, shm_size);
    close(fileDesc);
    if (isHost) {
        shm_unlink(shmName.c_str());
    }
}
