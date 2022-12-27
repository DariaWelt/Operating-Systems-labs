#ifndef LAB2_CLIENT_H
#define LAB2_CLIENT_H

#include <csignal>
#include <semaphore.h>
#include "../conn/IConn.hpp"
#include "../gui/gui.h"
#include "../conn/client_info.hpp"
#include "../utils/safeQueue.h"
#include "../gui/gui.h"

class Client {
public:

    static  Client &getInstance();
   
    void run();
    
    void terminate();
    void setHostPid(int hostPid);
    bool sendMessages(ConnInfo& info);
    bool getMessages(ConnInfo& info);
    static void guiSend(IConn::Message msg);
    static bool guiGet(IConn::Message *msg);
    static bool isRunning(void) { return getInstance()._isRunning; }
private:
    static Client _instance;
    SafeQueue _inputMsg;
    SafeQueue _outputMsg;
    ConnInfo info;
    GUI *_gui = nullptr;
    int _host_pid;

    bool _isRunning = true;

    void openConnection(ConnInfo& info);
    void processConn();
    
    explicit Client();
    std::string getMessage();
    static void handleSignal(int signum, siginfo_t *info, void *ptr);

    Client(Client &) = delete;
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
};


#endif //LAB2_CLIENT_H