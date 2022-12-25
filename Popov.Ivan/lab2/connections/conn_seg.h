#ifndef __CONN_SEG_H_
#define __CONN_SEG_H_

#include "connectionInterface.h"
#include <string>

class ConnectionSeg : public ConnectionInterface
{
  private:
    bool isHost;
    int segId;
    std::string segName;
    const uint seg_size = 1024;
    int shift = 0;
    void* segPtr = nullptr;
  public:
    ConnectionSeg(pid_t pid, bool isHost);

    void Update() {shift = 0;};
    void Open(size_t id, bool create) override;
    void Read(void *buf, size_t count) override;
    void Write(void *buf, size_t count) override;
    void Close(void) override;

    ~ConnectionSeg(void) = default;
};
#endif
