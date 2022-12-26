#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../client/client.h"
#include "../gui/gui.h"
#include "../connections/conn.h"
#include "../queueMsg.h"

class Host {
private:
    std::atomic<bool> isRunning = true;

    std::unique_ptr<Connection> conn;
    pid_t hostPid;
    pid_t clientPid;
    sem_t *hostSem;
    sem_t *clientSem;

    void connectionWork();

    bool prepare();
    bool readMsg();
    bool writeMsg();
    void connectionClose();

    MessageQueue messagesIn;
    MessageQueue messagesOut;
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);

    std::chrono::time_point<std::chrono::high_resolution_clock> lastMsgTime;
    static bool IsRun();
    static bool readWin(Message *msg);
    static void writeWin(Message msg);

    Host();
    Host(const Host&) = delete;
    Host(Host&&) = delete;
public:
    static Host& getInstance(){
        static Host hostInstance;
        return hostInstance;
    }

    void run();
    void stop();

    ~Host() = default;
};
