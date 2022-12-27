#include "clientHostConnection.h"
#include <syslog.h>
#include <unistd.h>
#include <cstring>

bool clientHostConnection::open(int id)
{
    _isClosed = false;
    _isSignalledTo = false;
    _clientId = id;
    if (pipe(_fd) == -1)
    {
        syslog(LOG_ERR, "Pipe creation failed: %s", strerror(errno));
        return false;
    }
    return true;
}

bool clientHostConnection::disconnect()
{
    if (close(_fd[0]) < 0 && close(_fd[1]) < 0)
    {
        syslog(LOG_ERR, "Disconnection of pipe failed: %s", strerror(errno));
        return false;
    }

    _isClosed = true;
    return true;
}

bool clientHostConnection::read(void *buf, size_t count)
{
    if (::read(_fd[0], buf, count) <= 0)
    {
        syslog(LOG_ERR, "Reading failed with error = %s", strerror(errno));
        return false;
    }
    return true;
}


bool clientHostConnection::write(void *buf, size_t count)
{
    if (::write(_fd[1], buf, count) == -1)
    {
        syslog(LOG_ERR, "Writing failed with error = %s", strerror(errno));
        return false;
    }
    return true;
}

int clientHostConnection::getClientId()
{
    return _clientId;
}

void clientHostConnection::signalTo()
{
    this->_isSignalledTo = true;
}

bool clientHostConnection::isSignalledTo() const
{
    return _isSignalledTo;
}

