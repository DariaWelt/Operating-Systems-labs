#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#include "../connection/message.h"
#include "futureWeather.h"
#include <semaphore.h>
#include <vector>


class clientHandler
{
public:
    clientHandler() = default;
    clientHandler(int id);
    void setClient(int pid);
    void start();
    bool openConnectionWeather();
    void setTid(pthread_t _tid);
    pthread_t getTid();
    int getId();
    connection getConn();
    clientHandler(clientHandler& c) = default;
    void terminate(int signum);


private:
    int _clientId;
    int _clientPid;
    bool _isAttached;
    bool _isClosed = false;
    connection _connWeather;
    sem_t *_semaphoreHost;
    sem_t *_semaphoreClient;

    std::string _semClientName;
    std::string _semHostName;

    pthread_t _tid;

    clientHandler& operator=(clientHandler& c);

    ~clientHandler(){};

    void msgFromDate(std::vector<int> _dateElems, message &msg);
    bool getDateMsg(message &msg);
    void killClient();
};
#endif