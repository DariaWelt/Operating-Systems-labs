#ifndef __CONNECTION_INTERFACE_H_
#define __CONNECTION_INTERFACE_H_

#include <sys/types.h>
#include <memory>
#include <stdexcept>
// Common interface for all connection types
class ConnectionInterface
{
public:
    // Function to create connection
    static std::unique_ptr<ConnectionInterface> Create(pid_t pid, bool isHost);
    // pure virtual functions
    virtual void Update() = 0;
    virtual void Open(size_t id, bool create) = 0;
    virtual void Read(void* buf, size_t count) = 0;
    virtual void Write(void* buf, size_t count) = 0;
    virtual void Close() = 0;

};
#endif
