#ifndef __CONN_SHM_H_
#define __CONN_SHM_H_

#include "connectionInterface.h"

#include <string>

class ConnectionShm : public ConnectionInterface
{
  private:
    bool isHost;
    std::string shmName;
    int shift = 0;
    const size_t shm_size = 1024;
    int fileDesc;
    void* shmPtr = nullptr;
  public:
    ConnectionShm(pid_t pid, bool isHost);

    void Update(void) { shift = 0; };
    void Open(size_t id, bool create) override;
    void Read(void *buf, size_t count) override;
    void Write(void *buf, size_t count) override;
    void Close(void) override;

    ~ConnectionShm(void) = default;
};
#endif
