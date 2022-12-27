#pragma once

#include <queue>
#include <mutex>
#include "../connections/connection.h"

// CONSTANT DEFINES
#define MAX_CHAR_LENGTH 300
struct Message {
    char text[MAX_CHAR_LENGTH];
};

// Protected message queue, which provides sending and recieving messages
class ConnectedQueue {
protected:
    std::queue<Message> q;
    mutable std::mutex mutex;
public:
    ConnectedQueue() = default;
    ConnectedQueue(const ConnectedQueue&) = delete;
    ConnectedQueue(ConnectedQueue&&) = delete;

    void Push(const Message& msg);
    bool Pop(Message* msg);

    size_t GetSize();

    bool PushFromConnection(Connection *conn);
    bool PopToConnection(Connection *conn);

    ~ConnectedQueue() = default;
};