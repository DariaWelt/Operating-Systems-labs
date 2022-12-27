#pragma once

#include "connection.h"

class FifoConnection : public Connection {
private:
    const std::string FIFO_CODE = "fifo";

    bool isHost;
    std::string fifoFilename;
    int fileId;
public:
    FifoConnection(pid_t pid, bool isHost) : isHost(isHost) {
        this->fifoFilename = "/tmp/fifo_" + std::to_string(pid);
    };

    void Open(size_t id, bool isHost) override;
    void Read(void* buf, size_t count) override;
    void Write(void* buf, size_t count) override;
    void Close() override;
    
    ~FifoConnection() = default;
};