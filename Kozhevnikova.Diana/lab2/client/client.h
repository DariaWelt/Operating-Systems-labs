#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../host/host.h"
#include "../gui/gui.h"
#include "../connections/conn.h"
#include "../queueMsg.h"

class Client{
private:
    std::atomic<bool> isRunning = true;

    std::unique_ptr<Connection> conn;
    pid_t hostPid;
    pid_t clientPid;
    sem_t *hostSem;
    sem_t *clientSem;

    void connectionWork();

    bool prepare(const pid_t& hostPid);
    bool readMsg();
    bool writeMsg();
    void connectionClose();

    MessageQueue messagesIn;
    MessageQueue messagesOut;
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);

    static bool IsRun();
    static bool readWin(Message *msg);
    static void writeWin(Message msg);

    Client();
    Client(const Client&) = delete;
    Client(Client&&) = delete;
public:
    static Client& getInstance(){
        static Client clientInstance;
        return clientInstance;
    }
    bool init(const pid_t& hostPid);
    void run();
    void stop();

    ~Client() = default;
};