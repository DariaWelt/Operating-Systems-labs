#ifndef __CONN_FIFO_H_
#define __CONN_FIFO_H_

#include "connectionInterface.h"
#include <string>

class ConnectionFifo : public ConnectionInterface
{
  private:
    std::string fifoName;
    bool isHost;
    int fileDesc;
  public:
    ConnectionFifo(pid_t pid, bool isHost);

    void Update(void) {};
    void Open(size_t id, bool create) override;
    void Read(void *buf, size_t count) override;
    void Write(void *buf, size_t count) override;
    void Close(void) override;

    ~ConnectionFifo(void) = default;
};
#endif
