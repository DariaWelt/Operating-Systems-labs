#pragma once
#include "../client/client.hpp"
//#include "client.hpp"
#include "../conn/conn.hpp"
//#include "conn.hpp"
#include "../conn/message.hpp"
//#include "message.hpp"
#include "../utils/utils.hpp"
//#include "utils.hpp"
#include "wolf.hpp"
#include <iostream>
#include <semaphore.h>
#include <signal.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>

class Host
{
public:
    Host()
    {
        semHostMsg = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        semClientMsg = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        if (semHostMsg == MAP_FAILED || semClientMsg == MAP_FAILED)
            throw std::runtime_error("Couldn't allocate shared memory for semaphores with mmap");
        if (sem_init(semHostMsg, 1, 0) == -1 || sem_init(semClientMsg, 1, 0) == -1)
            throw std::runtime_error("Couldn't initialize semaphores with sem_init");

        connection = Conn::create(true);
    }

    ~Host()
    {
        delete connection;
        if (sem_destroy(semHostMsg) == -1 || sem_destroy(semClientMsg) == -1)
        {
            std::cout << "Couldn't destroy semaphores" << std::endl;
            std::exit(1);
        }
        if (munmap(semHostMsg, sizeof(sem_t)) == -1 || munmap(semClientMsg, sizeof(sem_t)) == -1)
        {
            std::cout << "Couldn't free shared memory from semaphores" << std::endl;
            std::exit(1);
        }
    }

    void startGame()
    {
        addClient();

        while (!gameOver())
        {
            size_t wolfRoll = wolf.roll();
            std::cout << "Wolf rolled " << wolfRoll << std::endl;

            waitSem(semClientMsg, timeout);

            Message msg;
            connection->read(msg);
            std::cout << "Goat rolled " << msg.roll << std::endl;

            // Determine new goat state
            if (msg.alive)
                msg.alive = (size_t)abs((int)msg.roll - (int)wolfRoll) <= ALIVE_GAP ? true : false;
            else
                msg.alive = (size_t)abs((int)msg.roll - (int)wolfRoll) <= DEAD_GAP ? true : false;

            const std::string isAlive = msg.alive ? "True" : "False";
            std::cout << "Goat alive: " << isAlive << std::endl;

            if (!msg.alive)
                ++deathRow;
            else
                deathRow = 0;

            std::cout << "Death row: " << deathRow << std::endl;
            connection->write(msg);
            sem_post(semHostMsg);
            std::cout << "End of round" << std::endl << std::endl;
        }

        kill(childPid, SIGTERM);
    }

private:
    sem_t *semHostMsg, *semClientMsg;
    Conn* connection;
    Wolf wolf;
    size_t deathRow = 0;
    pid_t childPid;
    const size_t ALIVE_GAP = 70;
    const size_t DEAD_GAP = 20;
    const size_t timeout = 5;

    void addClient()
    {
        childPid = fork();
        if (childPid == 0)
        {
            Client client(semHostMsg, semClientMsg);
            client.run();
        }
        else
            if (childPid == -1)
                throw std::runtime_error("Couldn't create child proccess");
    }

    bool gameOver() const { return deathRow > 1 ? true : false; }
};