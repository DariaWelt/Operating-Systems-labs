#pragma once

#include "conn.h"

#include <string>

class ConnectionFifo : public Connection
{
public:
    ConnectionFifo(pid_t pid, Connection::Type type);
    ~ConnectionFifo();
    bool Read(void* buf, const std::size_t count) override;
    bool Write(const void* buf, const std::size_t count) override;

private:
    static constexpr char filePathTemplate_[] = "/tmp/fifo_";
    const std::string absPath_;
    int fileDescriptor_;
};