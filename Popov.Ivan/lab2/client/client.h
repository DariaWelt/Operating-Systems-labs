#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "../messageQueue/messageQueue.h"
#include "../gui/gui.h"
#include "../connections/connectionInterface.h"
#include <unistd.h>
#include <bits/types/siginfo_t.h>
#include <atomic>
#include <stdlib.h>
#include <semaphore.h>

// Client singleton class
class Client
{
  private:
    // Flag to check client status
    std::atomic<bool> isAlive = true;
    // Flag to check if client connected to host
    std::atomic<bool> isConnected = false;
    // Host pid
    std::atomic<pid_t> hostPid = -1;
    // Messages queue
    MessageQueue messagesIn;
    MessageQueue messagesOut;
    // GUI window
    GUI* gui = nullptr;

    std::unique_ptr<ConnectionInterface> conn;
    sem_t* hostSem;
    sem_t* clientSem;
    // Wait interval
    const int _waitInterval = 5;
    const int _connInterval = 5;
    const int _msgInterval = 60;
    const char* _windowName = "Chat client";

    // Function to handle signals
    static void SignalHandler(int signum, siginfo_t* info, void* ptr);

    // GUI functions
    static void GUISend(Message msg) { Client::GetInstance().messagesOut.Push(msg); };
    static bool GUIGet(Message* msg) { return Client::GetInstance().messagesIn.Pop(msg); };

    // Connection function
    void WorkConnection(void);
    bool OpenConnection(void);
    bool ReadConnection(void);
    bool WriteConnection(void);
    void CloseConnection(void);

    // Constructors and assignment operator are hidden
    Client();
    Client(Client const&) = delete;
    void operator=(Client const&) = delete;
  public:
    // Function to start host
    int Start(pid_t hostPid);
    // Function to stop host
    void Stop(void);
    // Function to get instance of Host class
    static Client& GetInstance() {
        static Client instance;
        return instance;
    };
    // Function to check if host is alive
    static bool IsAlive(void) { return GetInstance().isAlive.load(); };

    ~Client(void) = default;

};
#endif
