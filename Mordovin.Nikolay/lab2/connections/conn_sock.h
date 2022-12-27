#pragma once

#include <string>
#include <sys/socket.h>

#include "connection.h"

class SocketConnection : public Connection
{
public:
    SocketConnection(pid_t clientPid, bool isHost);
    void Open(size_t hostPid, bool isCreator) override;
    void Read(void* buf, size_t count) override;
    void Write(void* buf, size_t count) override;
    void Close(void) override;

    ~SocketConnection(void);

private:
    socklen_t m_host_socket, m_client_socket;
    std::string m_sock_name;
    bool m_isHost;
};