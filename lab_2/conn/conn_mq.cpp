#include "conn_mq.h"

Conn* Conn::createConnection() {
    return new ConnMq();
}

bool ConnMq::open(size_t hostPid, bool isCreator) {
    _isCreator = isCreator;
    _hostName = std::string("/lab2_conn_mq" + std::to_string(hostPid));

    if (_isCreator) {

        struct mq_attr mqAttr {0, 1, sizeof(Msg), 0};
        fd = mq_open(_hostName.c_str(), O_RDWR | O_CREAT, 0666, &mqAttr);
    } else {

        fd = mq_open(_hostName.c_str(), O_RDWR);
    }

    if (fd == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_mq : open");
        return false;
    }

    return true;

}

bool ConnMq::read(void* buf, size_t count) {

    if (mq_receive(fd, (char*)buf, count, 0) == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_mq : read");
        return false;
    }

    return true;
}

bool ConnMq::write(void* buf, size_t count) {

    if (mq_send(fd, (char*)buf, count, 0) == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_mq : write");
        return false;
    }

    return true;
}

bool ConnMq::close() {

    if (mq_close(fd) != 0) {

        syslog(LOG_ERR, "ERROR: problems with conn_mq : close");
        return false;
    } else if (_isCreator && mq_unlink(_hostName.c_str()) != 0) {

        syslog(LOG_ERR, "ERROR: problems with conn_mq : close : mq_unlink");
        return false;
    }

    return true;
}
