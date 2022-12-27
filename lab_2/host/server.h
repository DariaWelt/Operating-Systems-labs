#ifndef SERVER_H
#define SERVER_H
#include "../connection/connection.h"
#include "../connection/message.h"
#include "clientHandler.h"
#include <signal.h>
#include <vector>

class server
{
public:
    static server* getInstance();
    void start();
    void setNumOfClients(int num);
    std::vector<int> getDate();
    bool isSignalled();
    void signalGot();
    ~server(){};

private:
    int _clientsNum = 0;
    std::vector<int> _dateElems;
    bool _isSignalledTo = false;
    std::vector<clientHandler*> _clientThreads;

    server &operator=(server& s)
    {
        return s;
    }
    server();
    void createClient(clientHandler* handler);
    void sendDateMsg(std::vector<int> parcedDate);
    static void* execute(void* arg);
    static void signalHandler(int signum, siginfo_t *info, void *ptr);
    bool fromDate(std::string date, std::vector<int> &_dateElems);
    void terminate(int signum);
    void run();
};
#endif