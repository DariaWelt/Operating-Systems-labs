#ifndef CLIENTHOSTCONNECTION_H
#define CLIENTHOSTCONNECTION_H
#include "../connection/connection.h"
#include <pthread.h>


class clientHostConnection
{
public:
    bool disconnect();
    bool open(int id);
    bool read(void *buf, size_t size);
    bool write(void *buf, size_t size);
    int getClientId();
    bool isClosed() const { return _isClosed; };
    void signalTo();
    bool isSignalledTo() const;

private:
    bool _isSignalledTo = false;
    int _clientId;
    bool _isClosed = true;
    int _fd[2];
};
#endif