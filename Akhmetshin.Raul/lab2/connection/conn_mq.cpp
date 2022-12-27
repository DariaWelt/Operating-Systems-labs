#include "conn.h"

#include <cstring>
#include <fcntl.h>
#include <mqueue.h>
#include <syslog.h>

namespace {
    class ConnectionMq : public Connection {
    private:
        static std::string const pathname;

        std::string name;

    public:
        ConnectionMq(size_t id, bool create, size_t msg_size);
        ~ConnectionMq();

        bool Read(void* buffer, size_t count) override;
        bool Write(void* buffer, size_t count) override;

    };

    std::string const ConnectionMq::pathname = "/mq";
}

Connection* Connection::create(size_t id, bool create, size_t msg_size) {
    return new ConnectionMq(id, create, msg_size);
}

Connection::~Connection() {}


ConnectionMq::ConnectionMq(size_t id, bool create, size_t msg_size) {
    is_creater = create;
    name = pathname + std::to_string(id);

    int flags = O_RDWR;
    
    if (is_creater) {
        struct mq_attr attr;
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = msg_size;
        attr.mq_curmsgs = 0;

        flags |= O_CREAT;
        
        desc = mq_open(name.c_str(), flags, 0666, &attr);
    }
    else {
        desc = mq_open(name.c_str(), flags);
    }

    if (desc == -1) {
        syslog(LOG_ERR, "Can not open queue");
    }
}

ConnectionMq::~ConnectionMq() {
    if (mq_close(desc) == -1) {
        std::cout << "Bad close" << std::endl;
    }

    if (is_creater) {
        mq_unlink(name.c_str());
    }
}

bool ConnectionMq::Read(void* buffer, size_t count) {
    struct mq_attr attr;

    if (mq_getattr(desc, &attr) == -1) {
        syslog(LOG_ERR, "Can not get attr");
        return false;
    }

    if(mq_receive(desc, (char*)buffer, count, NULL) == -1) {
        syslog(LOG_ERR, "Can not read");
        return false;
    }

    return true;
}

bool ConnectionMq::Write(void* buffer, size_t count) {
    if (mq_send(desc, (char*)buffer, count, 0) == -1) {
        syslog(LOG_ERR, "Can not write");
        return false;
    }

    return true;
}
