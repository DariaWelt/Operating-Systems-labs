#include "messageQueue.h"
#include <syslog.h>

// Push to queue
void MessageQueue::Push(const Message& msg) {
    mutex.lock();
    queue.push(msg);
    mutex.unlock();
}

// Pop from queue
bool MessageQueue::Pop(Message* msg) {
    mutex.lock();
    if(!queue.empty()) {
        *msg = queue.front();
        queue.pop();
        mutex.unlock();
        return true;
    }
    mutex.unlock();
    return false;
}

// Push to connection
bool MessageQueue::PushConnection(ConnectionInterface* conn) {
    mutex.lock();
    conn->Update();
    uint msgAmount = 0;
    try {
        conn->Read((void*)&msgAmount, sizeof(uint));
        if (msgAmount == 0) {
            mutex.unlock();
            return true;
        }
    }
    catch (std::exception &err) {
        syslog(LOG_ERR, "%s", err.what());
        mutex.unlock();
        return false;
    }

    syslog(LOG_INFO, "Received %u msg", msgAmount);
    Message msg;
    for (uint i = 0; i < msgAmount; i++) {
        msg = {0};
        try {
            conn->Read((void*)&msg, sizeof(Message));
            std::string logStr = "Readed msg - " + std::string(msg.text);
            syslog(LOG_INFO, "%s", logStr.c_str());
            queue.push(msg);
        }
        catch (std::exception &err) {
            syslog(LOG_ERR, "%s", err.what());
            mutex.unlock();
            return false;
        }
    }
    mutex.unlock();
    return true;
}

// Pop from connection
bool MessageQueue::PopConnection(ConnectionInterface* conn) {
    mutex.lock();
    conn->Update();
    uint msgAnount = queue.size();
    conn->Write((void*)&msgAnount, sizeof(uint));
    while (!queue.empty()) {
        Message msg = queue.front();
        try {
            conn->Write((void*)&msg, sizeof(Message));
        }
        catch (std::exception &err) {
            syslog(LOG_ERR, "%s", err.what());
            mutex.unlock();
            return false;
        }
        queue.pop();
        syslog(LOG_INFO, "Sended message");
    }
    mutex.unlock();
    return true;
}
