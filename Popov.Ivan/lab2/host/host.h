#ifndef __HOST_H_
#define __HOST_H_

#include "../messageQueue/messageQueue.h"
#include "../gui/gui.h"
#include "../connections/connectionInterface.h"
#include <bits/types/siginfo_t.h>
#include <atomic>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

// Host singleton class
class Host
{
  private:

    // Flag to check host status
    std::atomic<bool> isAlive = true;
    // Client pid
    std::atomic<pid_t> clientPid = -1;
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
    const int _connInterval = 60;
    const int _msgInterval = 60;
    const char* _windowName = "Chat host";

    // Function to handle signals
    static void SignalHandler(int signum, siginfo_t* info, void* ptr);
    
    // GUI functions
    static void GUISend(Message msg) { Host::GetInstance().messagesOut.Push(msg); };
    static bool GUIGet(Message* msg) { return Host::GetInstance().messagesIn.Pop(msg); };

    // Connection function
    void WorkConnection(void);
    bool OpenConnection(void);
    bool ReadConnection(void);
    bool WriteConnection(void);
    void CloseConnection(void);

    // Constructors and assignment operator are hidden
    Host(void);
    Host(Host const&) = delete;
    void operator=(Host const&) = delete;
  public:
    // Function to start host
    int Start(void);
    // Function to stop host
    void Stop(void);
    // Function to get instance of Host class
    static Host& GetInstance(void) { 
      static Host hostInstance;
      return hostInstance; 
    };
    // Function to check if host is alive
    static bool IsAlive(void) { return GetInstance().isAlive.load(); };
    
    ~Host(void) = default;
    
};
#endif
