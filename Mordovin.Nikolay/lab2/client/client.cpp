#include <QApplication>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <thread>

#include "client.h"

void Client::SignalHandler(int signum, siginfo_t *info, void *ptr)
{
    switch (signum)
    {
    case SIGTERM:
        Client::GetInstance().isRunning = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "Client terminated");
        exit(EXIT_SUCCESS);
        return;
    case SIGUSR1:
        syslog(LOG_INFO, "Chat terminated");
        kill(Client::GetInstance().hostPid, SIGTERM);
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "Unknown command");
    }
}

Client::Client()
{
    struct sigaction sig
    {
    };

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Client::SignalHandler;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}

bool Client::Init(const pid_t &hostPid)
{
    syslog(LOG_INFO, "Initializing client");

    this->hostPid = hostPid;

    syslog(LOG_INFO, "Connecting semaphores");
    conn = Connection::createConnection(hostPid, false);
    if ((hostSem = sem_open("/Host-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0)) == SEM_FAILED)
    {
        syslog(LOG_ERR, "Failed to connect to host semaphore");
        return false;
    }

    if ((clientSem = sem_open("/Client-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0)) == SEM_FAILED)
    {
        sem_close(hostSem);
        syslog(LOG_ERR, "Failed to connect to client semaphore");
        return false;
    }

    try
    {
        conn->Open(hostPid, false);
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "%s", e.what());
        sem_close(hostSem);
        sem_close(clientSem);
        return false;
    }

    isRunning = true;
    syslog(LOG_INFO, "Successful client initialization");
    return true;
}

void Client::Run()
{
    syslog(LOG_INFO, "Starting client");

    std::thread connectionThread(&Client::StartMessageLoop, this);

    syslog(LOG_INFO, "Starting client GUI");
    std::string windowName = "Client";
    int argc = 1;
    char *args[] = {(char *)windowName.c_str()};
    QApplication app(argc, args);
    ChatWin window(windowName, WinWrite, WinRead, IsRun);
    window.show();
    app.exec();

    if (isRunning.load())
    {
        syslog(LOG_INFO, "Terminating client");
        isRunning = false;
    }
    connectionThread.join();
}

Client &Client::GetInstance()
{
    static Client clientInstance;
    return clientInstance;
}

void Client::StartMessageLoop()
{
    syslog(LOG_INFO, "Starting client message loop");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    while (isRunning.load())
    {
        // write all messages
        bool res = messagesOut.PopToConnection(conn.get());
        sem_post(hostSem);

        if (!res)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(32));

        timespec t;

        clock_gettime(CLOCK_REALTIME, &t);

        t.tv_sec += 5;

        int s = sem_timedwait(clientSem, &t);
        if (s == -1)
        {
            syslog(LOG_ERR, "Read semaphore timeout");
            isRunning = false;
            break;
        }

        messagesIn.PushFromConnection(conn.get());
    }

    syslog(LOG_INFO, "Message loop ended. Closing connection");
    conn->Close();
    sem_close(hostSem);
    sem_close(clientSem);
    kill(hostPid, SIGTERM);
}

bool Client::IsRun()
{
    return Client::GetInstance().isRunning.load();
}

bool Client::WinRead(Message *msg)
{
    return Client::GetInstance().messagesIn.Pop(msg);
}

void Client::WinWrite(Message msg)
{
    Client::GetInstance().messagesOut.Push(msg);
}
