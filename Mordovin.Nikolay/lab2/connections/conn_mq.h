#pragma once

#include <mqueue.h>
#include "connection.h"
#include "../messages/messages.h"

class MQConnection : public Connection {
private:
    const std::string MQ_CODE = "mq";
    const size_t MAX_SIZE = sizeof(Message);

    bool isCreator;
    std::string mqFilename;
    mqd_t mq;

public:
    MQConnection(pid_t pid, bool isHost) : isCreator(isHost) {
        this->mqFilename = "/mq_" + std::to_string(pid);
    };

    void Open(size_t id, bool isHost) override;
    void Read(void* buf, size_t count) override;
    void Write(void* buf, size_t count) override;
    void Close() override;

    ~MQConnection() = default;
};