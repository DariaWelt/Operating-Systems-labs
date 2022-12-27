#pragma once

#include <memory>
#include <string>

class Connection {
public:
    static std::unique_ptr<Connection> createConnection(pid_t pid, bool isHost);
    
    virtual void Open(size_t id, bool create) = 0;
    virtual void Read(void* buf, size_t count) = 0;
    virtual void Write(void* buf, size_t count) = 0;
    virtual void Close() = 0;
};