#include "futureWeather.h"
#include <syslog.h>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <iostream>

futureWeather* futureWeather::getInstance(int id)
{
    static futureWeather self(id);
    return &self;
}

futureWeather::futureWeather(int id)
{
    syslog(LOG_NOTICE, "Client id is: %d", id);
    _clientId = id;
    srand(time(NULL) + id);
    signal(SIGTERM, signalHandler);
}

futureWeather::futureWeather(futureWeather& other){}

futureWeather& futureWeather::operator=(futureWeather& other)
{
    return other;
}

bool futureWeather::openConnection()
{
    if ( _connection.open(_clientId, false) )
    {
        _semClientName = "client_" + std::to_string(_clientId);
        _semHostName = "host_" + std::to_string(_clientId);

        _semaphoreHost = sem_open(_semHostName.c_str(), 0);
        if (_semaphoreHost == SEM_FAILED)
        {
            syslog(LOG_ERR, "ERROR: client: can`t open host semaphore (%s) with error %s", _semHostName.c_str(), strerror(errno));
            return false;
        }

        syslog(LOG_NOTICE, "client: host semaphore opened (%s)", _semHostName.c_str());

        _semaphoreClient = sem_open(_semClientName.c_str(), 0);
        if (_semaphoreClient == SEM_FAILED)
        {
            syslog(LOG_ERR, "ERROR: client: can`t open client semaphore (%s) with error %s", _semClientName.c_str(), strerror(errno));
            return false;
        }

        syslog(LOG_NOTICE, "Host semaphore opened from client (%s)", _semClientName.c_str());

        return true;
    }
    return false;
}

void futureWeather::start()
{
    syslog(LOG_INFO, "Weather calculating started");
    message msg;
    message buf;
    while (true)
    {
        sem_wait(_semaphoreClient);
        if (_connection.read(&buf, sizeof(buf)))
        {
            msg.setTemp( getWeather( ) );
            _connection.write(&msg, sizeof(msg));
            sem_post(_semaphoreHost);
        }
    }
}


void futureWeather::terminate(int signum)
{
    syslog(LOG_NOTICE, "Termination of client(%d) started...", getpid());

    if (_connection.close())
    {
        if (_semaphoreHost != SEM_FAILED)
        {
            _semaphoreHost = SEM_FAILED;
            sem_close(_semaphoreHost);
        }

        if (_semaphoreClient != SEM_FAILED)
        {
            _semaphoreClient = SEM_FAILED;
            sem_close(_semaphoreClient);
        }
        exit(signum);
    }
    syslog(LOG_ERR, "Terminating error: %s", strerror(errno));
    exit(errno);
}


int futureWeather::getWeather() //here must be day, month and year, but they don't need here
{
    return rand() % 50 - 25;
}


void futureWeather::signalHandler(int signum)
{
    futureWeather *instance = futureWeather::getInstance(getpid());
    if(signum == SIGTERM)
    {
        instance->terminate(signum);
    }
}

void futureWeather::setConnection(connection _conn)
{
    _connection = _conn;
}
