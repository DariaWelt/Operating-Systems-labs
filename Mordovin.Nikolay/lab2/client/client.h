#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../host/host.h"
#include "../window/ChatWin.h"
#include "../connections/connection.h"
#include "../messages/messages.h"

class Client{
private:
    std::atomic<bool> isRunning = true;
    
    // connetcions
    std::unique_ptr<Connection> conn;
    pid_t hostPid;
    pid_t clientPid;
    sem_t *hostSem;
    sem_t *clientSem;

    void StartMessageLoop();

    // Signals and msgs managment
    ConnectedQueue messagesIn;
    ConnectedQueue messagesOut;
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);

    // Window managment
    static bool IsRun();
    static bool WinRead(Message *msg);
    static void WinWrite(Message msg);

    Client();
    Client(const Client&) = delete;
    Client(Client&&) = delete;
public:
    static Client& GetInstance();
    bool Init(const pid_t& hostPid);
    void Run();

    ~Client() = default;
};