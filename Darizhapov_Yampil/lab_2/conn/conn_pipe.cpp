#include "conn.hpp"
#include <iostream>
#include <stdexcept>
#include <unistd.h>

class ConnPipe : public Conn
{
public:
    ~ConnPipe()
    {
        if (close(hostFd[0]) == -1 || close(hostFd[1]) == -1 || close(clientFd[0]) == -1 || close(clientFd[1]) == -1)
        {
            std::cout << "Couldn't close pipe file descriptor" << std::endl;
            std::exit(1);
        }
    }

    void read(Message &msg) override
    {
        const int error = isHost ? ::read(clientFd[0], &msg, sizeof(msg)) : ::read(hostFd[0], &msg, sizeof(msg));
        if (error == -1)
            throw std::runtime_error("Couldn't read pipe");
    }

    void write(const Message &msg) override
    {
        const int error = isHost ? ::write(hostFd[1], &msg, sizeof(msg)) : ::write(clientFd[1], &msg, sizeof(msg));
        if (error == -1)
            throw std::runtime_error("Couldn't write to pipe");
    }

private:
    ConnPipe(int hostPipeFd[], int clientPipeFd[], bool isHost)
    {
        hostFd[0] = hostPipeFd[0];
        hostFd[1] = hostPipeFd[1];
        clientFd[0] = clientPipeFd[0];
        clientFd[1] = clientPipeFd[1];
        this->isHost = isHost;
    }

    friend Conn* Conn::create(bool isHost);

    int hostFd[2], clientFd[2];
    bool isHost;
};

Conn* Conn::create(bool isHost)
{
    static int hostFd[2], clientFd[2];
    if (isHost)
        if (pipe(hostFd) == -1 || pipe(clientFd) == -1)
            throw std::runtime_error("Couldn't create pipe");

    return new ConnPipe(hostFd, clientFd, isHost);
}