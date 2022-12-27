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

void Client::SignalHandler(int signum, siginfo_t* info, void *ptr) {
    switch (signum) {
    case SIGTERM:
        Client::getInstance().isRunning = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "INFO[Client] client terminated");
        exit(EXIT_SUCCESS);
        return;
    case SIGUSR1:
        syslog(LOG_INFO, "INFO[Client] chat terminated");
        kill(Client::getInstance().hostPid, SIGTERM);
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "INFO[Client] unknown command");
    }
}

Client::Client() {
    struct sigaction sig{};

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Client::SignalHandler;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}

bool Client::init(const pid_t& hostPid) {
    syslog(LOG_INFO, "INFO[Client]: initializing");
    isRunning = prepare(hostPid);

    if (isRunning)
        syslog(LOG_INFO, "INFO[Client]: All inited succesful");
    else
        syslog(LOG_INFO, "INFO[Client]: Cant init chat");

    return isRunning;
}

void Client::run() {
    syslog(LOG_INFO, "INFO[Client]: Client started run");
    
    std::thread connThread(&Client::connectionWork, this);
    std::string winName = "Client";
    int argc = 1;
    char* args[] = { (char*)winName.c_str() };
    QApplication app(argc, args);
    ChatWin window(winName, writeWin, readWin, IsRun);
    window.show();
    app.exec();
    stop();
    connThread.join();
}

void Client::stop() {
    if (isRunning.load()) {
        syslog(LOG_INFO, "INFO[Client] stop working");
        isRunning = false;
    }
}

bool Client::prepare(const pid_t& hostPid) {
    syslog(LOG_INFO, "INFO [Client]: start init connection");
    this->hostPid = hostPid;
    
    conn = Connection::create(hostPid, false);

    hostSem = sem_open("/Host-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (hostSem == SEM_FAILED) {
        syslog(LOG_ERR, "ERROR [Client]: cant connect to host sem");
        return false;
    }
    clientSem = sem_open("/Client-sem", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (clientSem == SEM_FAILED) {
        sem_close(hostSem);
        syslog(LOG_ERR, "ERROR [Client]: cant connect to client sem");
        return false;
    }

    try {
        conn->open(hostPid, false);
        Client::getInstance().isRunning = true;
        syslog(LOG_INFO, "INFO [Client]: connection initializing complete!");
        return true;
    }
    catch (std::exception &e) {
        syslog(LOG_ERR, "ERROR [Client]: %s", e.what());
        sem_close(hostSem);
        sem_close(clientSem);
        return false;
    }
}

void Client::connectionWork() {
    syslog(LOG_INFO, "INFO [Client]: Client starts working");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    while (isRunning.load()) {
        if (!writeMsg())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        if (!readMsg())
            break;
    }
    connectionClose();
}

bool Client::readMsg() {
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += 5;
        int s = sem_timedwait(clientSem, &t);
        if (s == -1)
        {
            syslog(LOG_ERR, "ERROR[Client] Read semaphore timeout");
            isRunning = false;
            return false;
        }
    }

    messagesIn.pushConnection(conn.get());
    return true;
}

bool Client::writeMsg() {
    bool res = messagesOut.popConnection(conn.get());
    sem_post(hostSem);
    return res;
}

void Client::connectionClose() {
    conn->close();
    sem_close(hostSem);
    sem_close(clientSem);
    kill(hostPid, SIGTERM);
}

bool Client::IsRun() {
    return Client::getInstance().isRunning.load();
}
    
bool Client::readWin(Message *msg) {
    return Client::getInstance().messagesIn.popMessage(msg);
}

void Client::writeWin(Message msg) {
    Client::getInstance().messagesOut.pushMessage(msg);
}
