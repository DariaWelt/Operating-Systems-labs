#include "conn_seg.h"

#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <filesystem>

ConnectionSeg::ConnectionSeg(pid_t pid, bool isHost) {
    this->isHost = isHost;
    segName = "/tmp/seg_" + std::to_string(pid);
}

std::unique_ptr<ConnectionInterface> ConnectionInterface::Create(pid_t pid, bool isHost) {
    return std::make_unique<ConnectionSeg>(pid, isHost);
}

void ConnectionSeg::Open(size_t id, bool create) {
    std::ofstream fs(segName);
    fs.close();
    key_t key = ftok(segName.c_str(), 1);
    if (key == -1) {
        throw std::runtime_error("Shmkey error");
    }
    if (isHost) {
        segId = shmget(key, seg_size, IPC_CREAT | 0666);
    } else {
        segId = shmget(key, seg_size, 0666);
    }
    if (segId == -1) {
        throw std::runtime_error("Creation error");
    }
    segPtr = shmat(segId, nullptr, 0);
}

void ConnectionSeg::Read(void *buf, size_t count) {
    if (count + shift > seg_size) {
        throw std::runtime_error("Reading error");
    }
    memcpy(buf, ((char *)segPtr + shift), count);
    shift += count;
}

void ConnectionSeg::Write(void *buf, size_t count) {
    if (count + shift > seg_size) {
        throw std::runtime_error("Writing error");
    }
    memcpy(((char *)segPtr + shift), buf, count);
    shift += count;
}

void ConnectionSeg::Close(void) {
    shmdt(segPtr);
    std::filesystem::remove(segName.c_str());
    if (isHost) {
        shmctl(segId, IPC_RMID, 0);
    }
}
