#include <QApplication>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <thread>

#include "host.h"

Host::Host()
{
    struct sigaction sig
    {
    };
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Host::SignalHandler;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
}

void Host::SignalHandler(int signum, siginfo_t *info, void *ptr)
{
    switch (signum)
    {
    case SIGTERM:
        Host::GetInstance().isRunning = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "Terminating host");
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "Unknown command");
    }
}

bool Host::Init()
{
    syslog(LOG_INFO, "Creating semaphores");
    hostPid = getpid();
    conn = Connection::createConnection(hostPid, true);
    
    if ((hostSem = sem_open("/Host-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0)) == SEM_FAILED)
    {
        syslog(LOG_ERR, "Host semaphore creation failed");
        return false;
    }
    
    if ((clientSem = sem_open("/Client-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0)) == SEM_FAILED)
    {
        sem_close(hostSem);
        syslog(LOG_ERR, "Client semaphore creation failed");
        return false;
    }

    syslog(LOG_INFO, "Client initialization");

    pid_t childPid = fork();

    if (childPid == 0)
    {
        clientPid = getpid();

        if (Client::GetInstance().Init(hostPid))
            Client::GetInstance().Run();
        else
        {
            syslog(LOG_ERR, "Client initialization error");
            return false;
        }
        exit(EXIT_SUCCESS);
    }

    try
    {
        Connection *raw = conn.get();
        raw->Open(hostPid, true);
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "%s", e.what());
        sem_close(hostSem);
        sem_close(clientSem);
        return false;
    }

    Host::GetInstance().isRunning = true;
    syslog(LOG_INFO, "Successful host initialization");
    return true;
}

void Host::Run()
{
    syslog(LOG_INFO, "Host initialization");
    if (Init() == false)
    {
        syslog(LOG_INFO, "Host initialization failed");
        isRunning = false;
        return;
    }

    std::thread connectionThread(&Host::StartMessageLoop, this);

    syslog(LOG_INFO, "Starting host GUI");
    std::string winName = "Host";
    int argc = 1;
    char *args[] = {(char *)winName.c_str()};
    QApplication app(argc, args);
    ChatWin window(winName, WinWrite, WinRead, IsRun);
    window.show();
    app.exec();

    if (isRunning.load())
    {
        syslog(LOG_INFO, "Terminating host");
        isRunning = false;
    }
    connectionThread.join();
}

Host &Host::GetInstance()
{
    static Host hostInstance;
    return hostInstance;
}

void Host::StartMessageLoop()
{
    lastMsgTime = std::chrono::high_resolution_clock::now();

    while (isRunning.load())
    {
        double minutes_passed = std::chrono::duration_cast<std::chrono::minutes>(
                                    std::chrono::high_resolution_clock::now() - lastMsgTime)
                                    .count();

        if (minutes_passed >= 1)
        {
            syslog(LOG_INFO, "Chat not used for a minute. Killing process");
            isRunning = false;
            break;
        }

        timespec t;

        clock_gettime(CLOCK_REALTIME, &t);

        t.tv_sec += 5;

        int s = sem_timedwait(hostSem, &t);
        if (s == -1)
        {
            syslog(LOG_ERR, "Read semaphore timeout");
            isRunning = false;
            break;
        }

        if (messagesIn.PushFromConnection(conn.get()) == false)
        {
            isRunning = false;
            break;
        }
        else if (messagesIn.GetSize() > 0)
            lastMsgTime = std::chrono::high_resolution_clock::now();

        bool res = messagesOut.PopToConnection(conn.get());
        sem_post(clientSem);
        if (!res)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(32));
    }

    conn->Close();
    sem_close(hostSem);
    sem_close(clientSem);
    kill(clientPid, SIGTERM);
}

bool Host::IsRun()
{
    return Host::GetInstance().isRunning.load();
}

bool Host::WinRead(Message *msg)
{
    return Host::GetInstance().messagesIn.Pop(msg);
}

void Host::WinWrite(Message msg)
{
    Host::GetInstance().messagesOut.Push(msg);
}

int main(int argc, char *argv[]) {
    openlog("Chat log", LOG_NDELAY | LOG_PID, LOG_USER);

    try {
        Host::GetInstance().Run();
    } catch (std::exception &e) {
        syslog(LOG_ERR, "%s. Closing chat...", e.what());
    }

    closelog();
    return 0;
}