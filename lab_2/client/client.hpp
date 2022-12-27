#pragma once
#include "conn.hpp"
#include "goat.hpp"
#include "message.hpp"
#include "utils.hpp"
#include <semaphore.h>
#include <stdexcept>

class Client
{
public:
    Client(sem_t* semHost, sem_t* semClient)
    {
        semHostMsg = semHost;
        semClientMsg = semClient;
        connection = Conn::create(false);
    }

    void run()
    {
        while (true)
        {
            Message msg = {goat.roll(), goat.getStatus()};
            connection->write(msg);
            sem_post(semClientMsg);

            waitSem(semHostMsg, timeout);
            connection->read(msg);
            goat.setStatus(msg.alive);
        }
    }

private:
    Goat goat;
    sem_t *semClientMsg, *semHostMsg;
    Conn* connection;
    const size_t timeout = 5;
};