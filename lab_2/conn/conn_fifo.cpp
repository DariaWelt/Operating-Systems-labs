#include "conn_fifo.h"

Conn* Conn::createConnection() {
    return new ConnFifo();
}

bool ConnFifo::open(size_t hostPid, bool isCreator) {
    _isCreator = isCreator;
    _hostName = std::string("/tmp/named_pipe" + std::to_string(hostPid));

    if (_isCreator) {

        unlink(_hostName.c_str());
        if (mkfifo(_hostName.c_str(), 0666) == -1) {

            syslog(LOG_ERR, "ERROR: problems with conn_mkfifo : open");
            return false;
        }
    }

    syslog(LOG_INFO, "INFO : MKFIFO : created");

    if ((fd = ::open(_hostName.c_str(), O_RDWR)) == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_mkfifo : open");
        return false;
    }

    syslog(LOG_INFO, "INFO : MKFIFO : opened");

    return true;
}

bool ConnFifo::read(void* buf, size_t count) {

    if (::read(fd, buf, count) == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_mkfifo : read");
        return false;
    }

    return true;
}

bool ConnFifo::write(void* buf, size_t count) {

    if (::write(fd, buf, count) == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_mkfifo : write");
        return false;
    }

    return true;
}

bool ConnFifo::close() {

    if (::close(fd) < 0 || (_isCreator && unlink(_hostName.c_str()) == -1)) {

        syslog(LOG_ERR, "ERROR: problems with conn_mkfifo : close");
        return false;
    }

    syslog(LOG_INFO, "INFO: MKFIFO : closed");

    return true;
}