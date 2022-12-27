#include "conn_mq.h"

std::unique_ptr<Connection> Connection::createConnection(pid_t pid, bool isHost) {
    return std::make_unique<MQConnection>(pid, isHost);
}

void MQConnection::Open(size_t id, bool isHost) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    if (isHost)
        mq = mq_open(mqFilename.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    else
        mq = mq_open(mqFilename.c_str(), O_RDWR | O_EXCL);

    if ((mqd_t)-1 == mq) {
        throw std::runtime_error("error while opening mqueue");
    }
}

void MQConnection::Read(void* buf, size_t count) {
    if (count > MAX_SIZE)
        throw std::invalid_argument("Read error");

    mq_receive(mq, (char *)buf, MAX_SIZE, nullptr);
}

void MQConnection::Write(void* buf, size_t count) {
    if (count > MAX_SIZE)
        throw std::invalid_argument("Write error");

    mq_send(mq, (const char *)buf, MAX_SIZE, 0);
}

void MQConnection::Close() {
    mq_close(mq);
}