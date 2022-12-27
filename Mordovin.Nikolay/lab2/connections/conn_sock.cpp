#include <fcntl.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/syslog.h>

#include "conn_sock.h"

std::unique_ptr<Connection> Connection::createConnection(pid_t clientPid, bool isHost)
{
    return std::make_unique<SocketConnection>(clientPid, isHost);
}

SocketConnection::SocketConnection(pid_t clientPid, bool isHost)
{
  m_isHost = isHost;
  m_sock_name = "/tmp/sock_" + std::to_string(clientPid);
}

void SocketConnection::Open(size_t hostPid, bool isCreator)
{
  struct sockaddr_un add;
  add.sun_family = AF_UNIX;
  strncpy(add.sun_path, m_sock_name.c_str(), sizeof(add.sun_path) - 1);
  if (m_isHost)
  {
    m_host_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (m_host_socket < 0)
      throw ("host socket creation error");
    if (bind(m_host_socket, (struct sockaddr *) &add, sizeof(add)) < 0)
      throw ("bind socket error");
    if (listen(m_host_socket, 1) < 0)
      throw ("listen error");
    m_client_socket = accept(m_host_socket, NULL, NULL);
    if (m_client_socket < 0)
      throw ("accept client socket error");
  }
  else
  {
    m_client_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (m_client_socket < 0)
      throw ("client socket creation error");
    if (connect(m_client_socket, (const struct sockaddr *) &add, sizeof(struct sockaddr_un)) == -1)
      throw ("connect sockets error");
  }
}

void SocketConnection::Read(void* buf, size_t count)
{
  if (recv(m_client_socket, buf, count, 0) < 0)
    throw ("Read socket error");
}

void SocketConnection::Write(void* buf, size_t count)
{
  if (send(m_client_socket, buf, count, MSG_NOSIGNAL) < 0)
    throw ("send socket error");
}

void SocketConnection::Close(void)
{
  if (m_isHost)
  {
    close(m_client_socket);
    close(m_host_socket);
    unlink(m_sock_name.c_str());
  }
  else
  {
    close(m_client_socket);
  }
}

SocketConnection::~SocketConnection(void)
{
}