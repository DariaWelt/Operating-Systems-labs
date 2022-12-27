#include "conn_seg.h"

Conn* Conn::createConnection() {
    return new ConnSeg();
}

bool ConnSeg::open(size_t hostPid, bool isCreator) {
    _isCreator = isCreator;
    int shmflag = 0666;

    if (_isCreator) {
        shmflag |= IPC_CREAT;
    }

    fd = shmget(hostPid, sizeof(Msg), shmflag);

    if (fd == -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_seg : open");
        return false;
    }

    return true;
}

bool ConnSeg::read(void* buf, size_t count) {

    Msg* msg = (Msg*)shmat(fd, nullptr, 0);

    if (msg == (Msg*) -1) {

        syslog(LOG_ERR, "ERROR: problems with conn_seg : read");
        return false;
    }

    *((Msg* ) buf) = *msg;

    shmdt(msg);

    return true;
}

bool ConnSeg::write(void* buf, size_t count) {

    Msg* ptr = (Msg *)shmat(fd, nullptr, 0);

    if (ptr == (Msg *)-1) {

        syslog(LOG_ERR, "ERROR: problems with conn_seg : write");
        return false;
    }

    *ptr = *((Msg *)buf);

    shmdt(ptr);

    return true;
}

bool ConnSeg::close() {

    if (_isCreator && shmctl(fd, IPC_RMID, nullptr) < 0) {

        syslog(LOG_ERR, "ERROR: problems with conn_seg : close");
        return false;
    }

    return true;
}

