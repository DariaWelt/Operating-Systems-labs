#include "connection.h"
#include "message.h"
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static std::string shmName = "/shm";
bool connection::open(size_t id, bool create)
{
    _isCreated = create;
    _connName = shmName + std::to_string(id);
    int mode = 0666;
    int fd;

    if (_isCreated)
    {
        fd = shm_open(_connName.c_str(), O_CREAT | O_RDWR, mode);

        if (fd == -1)
        {
            syslog(LOG_ERR, "Error in opening shared memory: %s", strerror(errno));
            return false;
        }

        ftruncate(fd, sizeof(message));
    }
    else
    {
        fd = shm_open(_connName.c_str(), O_RDWR, mode);

        if (fd == -1)
        {
            syslog(LOG_ERR, "Error in opening shared memory: %s", strerror(errno));
            return false;
        }
    }

    _fileDescr = (int*)mmap(0, sizeof(message), PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

    if (_fileDescr == MAP_FAILED)
    {
        syslog(LOG_ERR, "ERROR: mmap failed, error = %s", strerror(errno));
        return false;
    }
    return true;
}

bool connection::close()
{
    munmap(_fileDescr, sizeof(message));
    shm_unlink(_connName.c_str());
    syslog(LOG_INFO, "Shm_memory connection is closed.");
    return true;
}

bool connection::read(void *buf, size_t count)
{
    if (count > sizeof(message))
    {
        syslog(LOG_ERR, "Incorrect message size.");
        return false;
    }
    return (memcpy(buf, _fileDescr, count) != nullptr);
}

bool connection::write(void *buf, size_t count)
{
    if (count > sizeof(message))
    {
        syslog(LOG_ERR, "Incorrect message size.");
        return false;
    }

    return (memcpy(_fileDescr, buf, count) != nullptr);
}
