#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../client/client.h"
#include "../window/ChatWin.h"
#include "../connections/connection.h"
#include "../messages/messages.h"


// 'Host' managment class: singleton
class Host {
private:
    // Instance
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
    std::chrono::time_point<std::chrono::high_resolution_clock> lastMsgTime;
    static bool IsRun();
    static bool WinRead(Message *msg);
    static void WinWrite(Message msg);

    bool Init();

    // constructions 
    Host();
    Host(const Host&) = delete;
    Host(Host&&) = delete;
public:
    static Host& GetInstance();
    void Run();

    ~Host() = default;
};
