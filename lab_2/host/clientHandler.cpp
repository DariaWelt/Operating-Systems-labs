#include "clientHandler.h"
#include "server.h"
#include <fcntl.h>
#include <cstring>
#include <csignal>
#include <semaphore.h>
#include <syslog.h>
#include <iostream>

clientHandler::clientHandler(int id)
{
    _clientId = id;
}

clientHandler& clientHandler::operator=(clientHandler& c)
{
    return c;
}

void clientHandler::setClient(int pid)
{
    _clientPid = pid;
    syslog(LOG_INFO, "Setting client with pid: %d", pid);
    _isAttached = (_clientPid != 0);
}

connection clientHandler::getConn()
{
    return _connWeather;
}

bool clientHandler::openConnectionWeather()
{
    if (_connWeather.open(_clientId, true))
    {
        _semClientName = "client_" + std::to_string(_clientId);
        _semHostName = "host_" + std::to_string(_clientId);

        _semaphoreHost = sem_open(_semHostName.c_str(), O_CREAT, 0666, 0);

        if (_semaphoreHost == SEM_FAILED)
        {
            syslog(LOG_ERR, "ERROR: handler: can`t open host semaphore error = %s", strerror(errno));
            return false;
        }

        syslog(LOG_NOTICE, "handler: host semaphore created (%s)", _semHostName.c_str());

        _semaphoreClient = sem_open(_semClientName.c_str(), O_CREAT, 0666, 0);

        if (_semaphoreClient == SEM_FAILED)
        {
            syslog(LOG_ERR, "ERROR: handler: can`t open client semaphore error = %s", strerror(errno));
            return false;
        }

        syslog(LOG_NOTICE, "handler: client semaphore created (%s)", _semClientName.c_str());
        return true;
    }
    return false;
}

void clientHandler::setTid(pthread_t tid)
{
    _tid = tid;
}

pthread_t clientHandler::getTid()
{
    return _tid;
}

int clientHandler::getId()
{
    return _clientId;
}

void clientHandler::killClient()
{
    if (_isAttached)
    {
        syslog(LOG_NOTICE, "Kill client");
        kill(_clientPid, SIGTERM);
    }
}

void clientHandler::terminate(int signum)
{
    syslog(LOG_NOTICE, "Termination of handler id %d. Is closed = %i", _clientId, _isClosed);
    if (_isClosed)
    {
        return;
    }

    _isClosed = true;
    killClient();

    if (!_connWeather.close())
    {
        syslog(LOG_ERR, "Terminating error connection: %s", strerror(errno));
    }

    if (_semaphoreHost != SEM_FAILED)
    {
        _semaphoreHost = SEM_FAILED;

        if (sem_unlink(_semHostName.c_str()) != 0)
        {
            syslog(LOG_ERR, "Terminating error semHostName: %s", strerror(errno));
        }
    }

    if (_semaphoreClient != SEM_FAILED)
    {
        _semaphoreClient = SEM_FAILED;

        if (sem_unlink(_semClientName.c_str()) != 0)
        {
            syslog(LOG_ERR, "Terminating error semClientName: %s", strerror(errno));
        }
    }
    syslog(LOG_NOTICE, "Handler %d was killed", _clientId);
}

bool clientHandler::getDateMsg(message &msg)
{
    server* curServer = server::getInstance();

    if (!curServer)
    {
        syslog(LOG_ERR, "Impossible to reach server");
        return false;
    }

    std::vector<int> _dateElems = curServer->getDate();
    msgFromDate(_dateElems, msg);
    return true;
}

void clientHandler::start()
{
    struct timespec ts;
    int res;
    message msg;
    message buf;
    server* curServer = server::getInstance();

    if (!curServer)
    {
        syslog(LOG_ERR, "Impossible to reach server");
        return;
    }

    while (true)
    {
        if (curServer->isSignalled())
        {
            syslog(LOG_INFO, "Getting msg...");
            getDateMsg(msg);
            std::cout << "Message got! ";
            syslog(LOG_INFO, "Message got!");

            if (_connWeather.write(&msg, sizeof(msg)))
            {
                std::cout << "Wait for response..." << std::endl;
                sem_post(_semaphoreClient);
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += timeout * 1000;
                res = sem_timedwait(_semaphoreHost, &ts);

                if (res == -1)
                {
                    killClient();
                    terminate(errno);
                    return;
                }
                else
                {
                    if (_connWeather.read(&buf, sizeof(buf)))
                    {
                        std::cout << "ANSWER: date = " << msg.getDay() << "/" << msg.getMonth() << "/"<< msg.getYear();
                        std::cout << ", RESPONSE: temperature = " << buf.getTemp() << std::endl;
                    }
                }
            }
            curServer->signalGot();
        }
    }
}

void clientHandler::msgFromDate(std::vector<int> _dateElems, message &msg)
{
    msg.setDay(_dateElems.at(0));
    msg.setMonth(_dateElems.at(1));
    msg.setYear(_dateElems.at(2));
}
