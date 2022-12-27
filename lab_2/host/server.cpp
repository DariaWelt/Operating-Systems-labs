#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <syslog.h>
#include <sstream>


server* server::getInstance()
{
    static server self;
    return &self;
}

void server::setNumOfClients(int num)
{
    _clientsNum = num;
    _clientThreads.clear();
    _clientThreads = std::vector<clientHandler*>();
}

server::server()
{
    _isSignalledTo = false;
    struct sigaction act{};
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signalHandler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, nullptr);
}

void server::createClient(clientHandler* handler)
{
    int _clientPid;
    _clientPid = fork();
    if (_clientPid == 0)
    {
        futureWeather* client = futureWeather::getInstance(handler->getId());
        client->setConnection(handler->getConn());
        if(!client->openConnection())
        {
            syslog(LOG_ERR, "Impossible to connect handler and client");
            return;
        }
        client->start();
        syslog(LOG_INFO, "Client created");
        return;
    }
    handler->setClient(_clientPid);
}

void* server::execute(void *arg)
{
    auto* handler = (clientHandler*)arg;
    handler->start();
    return nullptr;
}

void server::start()
{
    syslog(LOG_INFO, "Creation of all clients started in server");
    for (int i = 0; i < _clientsNum; i++)
    {
        auto *handler = new clientHandler(i);
        if(!handler->openConnectionWeather())
        {
            std::cout << "Error in connection weather" << std::endl;
            syslog(LOG_ERR, "Error in connection establishment: %s", strerror(errno));
            return;
        }
        createClient(handler);
        _clientThreads.push_back(handler);

        pthread_t _tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        handler->setTid(_tid);
        void *(*function)(void *) = execute;
        pthread_create(&_tid, &attr, function, handler);
    }
    run();
}

void server::signalHandler(int signum, siginfo_t *info, void *ptr)
{
    static server* instance = getInstance();
    switch (signum)
    {
        case SIGTERM:
        {
            instance->terminate(signum);
            break;
        }
    }
}

void server::terminate(int signum)
{
      syslog(LOG_INFO, "Starting clients terminating...");
      for( clientHandler *client : _clientThreads)
       {
           client->terminate(signum);
       }
}

void server::run()
{
    std::string recievedDate;
    std::vector<int> parcedDate;
    bool isIncorrect, isTerminate = false;
    do
    {
        do
        {
            recievedDate = "";
            parcedDate.clear();

            if(!_isSignalledTo)
            {
                if (isIncorrect && recievedDate != "")
                {
                    std::cout << "Incrorrect format: try again!" << std::endl;
                    isIncorrect = false;
                }

                std::cout << "Enter the date in format: Day/Month/Year OR enter q for exit."<< std::endl;
                std::cin >> recievedDate;
            }

            if (recievedDate == "q" || recievedDate == "Q")
            {
                syslog(LOG_INFO, "Terminating server...");
                isTerminate = true;
                terminate(EXIT_SUCCESS);
            }
            else
            {
                isIncorrect = !fromDate(recievedDate, parcedDate);
            }

        }while (isIncorrect && !isTerminate);

        if(!isTerminate)
        {
            sendDateMsg(parcedDate);
        }

    }while (!isTerminate);
}

void server::sendDateMsg(std::vector<int> parcedDate)
{
    _isSignalledTo = true;
    _dateElems = parcedDate;
}

bool server::fromDate(std::string date, std::vector<int> &_dateElems)
{
    if(date == "")
    {
        return false;
    }

    char delim = '/';
    std::stringstream strStream(date);
    std::string item;

    while(std::getline(strStream, item, delim))
    {
        _dateElems.push_back(std::stoi(item));
    }

    if (_dateElems.size() < 3)
    {
        syslog(LOG_ERR, "Incorrect format: not enough numbers. Got only %zu.", _dateElems.size());
        std::cout << "Incorrect format: not enough numbers. Got only " << _dateElems.size()  << std::endl;;
        return false;
    }

    int year = _dateElems.at(2);
    if (year <= 0)
    {
        syslog(LOG_ERR, "Incorrect date: year does not exist.");
        std::cout << "Incorrect date: year does not exist." << std::endl;
        return false;
    }

    int month = _dateElems.at(1);
    if (month <= 0 || month > 12)
    {
        syslog(LOG_ERR, "Incorrect date: month does not exist.");
        std::cout << "Incorrect date: month does not exist." << std::endl;
        return false;
    }

    int day = _dateElems.at(0);
    int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0)
    {
        days_in_month[1] = 29;
    }

    if ( day <= 0 || day > days_in_month[month - 1] )
    {
        syslog(LOG_ERR, "Incorrect date: day does not exist.");
        std::cout << "Incorrect date: day does not exist." << std::endl;
        return false;
    }

    return true;
}

std::vector<int> server::getDate()
{
    return _dateElems;
}

bool server::isSignalled()
{
    return _isSignalledTo;
}

void server::signalGot()
{
    _isSignalledTo = false;
}