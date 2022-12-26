#ifndef HOST_H
#define HOST_H

#include <semaphore.h>
#include <csignal>
#include <string>
#include "../conn/IConn.hpp"
#include "../conn/client_info.hpp"
#include "../gui/gui.h"
#include "../utils/safeQueue.h"
#include <atomic>

class Host {
public:
    static Host& getInstance();

    void openConn(ConnInfo &info);
    void start();
    void processConn();
    void terminate();
    bool sendMessages(ConnInfo& info);
    bool getMessages(ConnInfo& info);
    static void guiSend(IConn::Message msg);
    static bool guiGet(IConn::Message *msg);
    static bool isRunning(void) { return getInstance()._isRunning.load(); }
private:
    SafeQueue _inputMsg;
    SafeQueue _outputMsg;
    GUI *_gui = nullptr;
    int _clientPid = -1;
    ConnInfo info;
    std::atomic<bool> _isRunning = false;
    bool _isCreatedConn = false;
    static void handleSignal(int signum, siginfo_t *info, void *ptr);

    Host();
    Host(Host &) = delete;
    Host(const Host &) = delete;
    Host &operator=(const Host &) = delete;
};


#endif //HOST_H