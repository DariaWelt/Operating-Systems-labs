#include "connection.h"
#include "message.h"
#include <cerrno>
#include <mqueue.h>
#include <syslog.h>
#include <cstring>

static std::string mqName = "/mq";

bool connection::open(size_t id, bool create)
{
    _fileDescr = (int*)malloc(sizeof(int));
    _isCreated = create;
    _connName = mqName + std::to_string(id);
    int mode = 0666;

    if (_isCreated)
    {
        struct mq_attr attr = {0, 1, sizeof(message), 0, {0}};
        (*_fileDescr) = mq_open(_connName.c_str(), O_RDWR | O_CREAT, mode, &attr);
    }
    else
    {
        (*_fileDescr) = mq_open(_connName.c_str(), O_RDWR);
    }
    if (*_fileDescr == -1)
    {
        syslog(LOG_ERR, "Cannot establish connection with id = %zu, error = %s", id, strerror(errno));
        return false;
    }
    syslog(LOG_INFO, "connection with id %lu established successfully", id);
    return true;
}

bool connection::close()
{
    if (!mq_close(*_fileDescr))
    {
        free(_fileDescr);
        syslog(LOG_INFO, "connection is closed");
        return (!_isCreated || !(mq_unlink(_connName.c_str())));
    }

    syslog(LOG_ERR, "connection closing error");
    return false;
};

bool connection::read(void *buf, size_t count)
{
    if (mq_receive(*_fileDescr, (char *) buf, count, nullptr) == -1)
    {
        syslog(LOG_ERR, "Problems with reading: %s", strerror(errno));
        return false;
    }
    return true;
}

bool connection::write(void *buf, size_t count)
{
    if (mq_send(*_fileDescr, (char *) buf, count, 0) == -1)
    {
        syslog(LOG_ERR, "Problems with writing: %s", strerror(errno));
        return false;
    }
    return true;
}

