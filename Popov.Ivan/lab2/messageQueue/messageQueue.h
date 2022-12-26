#ifndef __MESSAGE_QUEUE_H_
#define __MESSAGE_QUEUE_H_

#include <queue>
#include <mutex>
#include "../connections/connectionInterface.h"

#define MSG_MAX_SIZE 256

struct Message {
    char text[MSG_MAX_SIZE];
};

// Message queue for storing chat messages
class MessageQueue
{
  private:
    std::queue<Message> queue;
    mutable std::mutex mutex;
  public:
    // Push to queue
    void Push(const Message& msg);
    // Pop from queue
    bool Pop(Message* msg);
    // Push to connection
    bool PushConnection(ConnectionInterface* conn);
    // Pop from connection
    bool PopConnection(ConnectionInterface* conn);

};
#endif
