#include "host.h"

#include <sys/syslog.h>
#include <csignal>
#include <cstring>
#include <thread>
#include <chrono>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    openlog("Chat host", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);
    int returnCode = Host::GetInstance().Start();

    closelog();
    return returnCode;
}

Host::Host(void) {
    struct sigaction sig{};
    std::memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = SignalHandler;
    sig.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}

// Function to handle signals
void Host::SignalHandler(int signum, siginfo_t* info, void* ptr) {
    switch (signum) {
        case SIGUSR1:
            syslog(LOG_INFO, "Client with pid %d is trying to connect", info->si_pid);
            if (Host::GetInstance().clientPid == -1) {
                Host::GetInstance().clientPid = info->si_pid;
            } else {
                syslog(LOG_INFO, "Host can't accept anoter client right now");
            }
            break;
        case SIGTERM:
            Host::GetInstance().Stop();
            break;
    }
}

// Function to start host
int Host::Start(void) {
    syslog(LOG_INFO, "Host started");
    isAlive = true;
    clientPid = -1;
    try {
        std::thread connectionThread(&Host::WorkConnection, this);


        gui = new GUI("Host", GUIGet, GUISend, IsAlive);
        gui->Run();
        delete gui;
    

        Stop();
        connectionThread.join();
    } catch (std::exception& err) {
        syslog(LOG_ERR, "%s", err.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Function to stop host
void Host::Stop(void) {
    if (isAlive.load()) {
        syslog(LOG_INFO, "Host stopped working");
        isAlive = false;
    }
}

void Host::WorkConnection(void) {
    printf("host pid = %i\n", getpid());
    auto lastTimeCheckClient = std::chrono::high_resolution_clock::now();
    gui->SetConnected(false);
    while (isAlive.load()) {
        if (clientPid.load() == -1) {
            double timePassed = std::chrono::duration_cast<std::chrono::minutes>(
                std::chrono::high_resolution_clock::now() - lastTimeCheckClient).count();
            if (timePassed >= _connInterval) {
                Stop();
            }
            continue;
        }
        lastTimeCheckClient = std::chrono::high_resolution_clock::now();

        if (!OpenConnection()) {
            continue;
        }
        auto lastTimeCheckMsg = std::chrono::high_resolution_clock::now();
        gui->SetConnected(true);
        while (isAlive.load()) {
            double timePassed = std::chrono::duration_cast<std::chrono::minutes>(
                std::chrono::high_resolution_clock::now() - lastTimeCheckMsg).count();  
            if (timePassed >= _msgInterval) {
                syslog(LOG_INFO, "Shutting down client for silence");
                kill(clientPid, SIGTERM);
                clientPid = -1;
                break;
            } 
        
            if (!ReadConnection()) {
                break;
            }
            if (!WriteConnection()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        CloseConnection();
        gui->SetConnected(false);
    }
    if (clientPid != -1) {
        kill(clientPid, SIGTERM);
    }

}

bool Host::OpenConnection(void) {
    syslog(LOG_INFO, "Opening connection");
    conn = ConnectionInterface::Create(clientPid.load(), true);

    std::string semNameHost = "/host_" + std::to_string(clientPid.load());
    std::string semNameClient = "/client_" + std::to_string(clientPid.load());
    hostSem = sem_open(semNameHost.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (hostSem == SEM_FAILED) {
        syslog(LOG_ERR, "Error while creating host semaphore");
        return false;
    }
    clientSem = sem_open(semNameClient.c_str(),  O_CREAT | O_EXCL, 0777, 0);
    if (clientSem == SEM_FAILED) {
        sem_close(hostSem);
        syslog(LOG_ERR, "Error while creating client semaphore");
        clientPid = -1;
        return false;
    }

    syslog(LOG_INFO, "Semaphores created");

    if (kill(clientPid.load(), SIGUSR1) != 0) {
        syslog(LOG_ERR, "Cannot send signal to client");
    }

    try {
        conn.get()->Open(0, false);
    }
    catch (std::exception &err) {
        sem_close(hostSem);
        sem_close(clientSem);
        syslog(LOG_ERR, "Error while opening connection: %s", err.what());
        clientPid = -1;
        return false;
    }

    syslog(LOG_INFO, "Opened connection");
    return true;
}

bool Host::ReadConnection(void) {
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);

        t.tv_sec += _waitInterval;
        int s = sem_timedwait(hostSem, &t);
        if (s == -1) {
            syslog(LOG_ERR, "Host semaphore timeout");
            isAlive = false;
            return false;
        }
    }
    messagesIn.PushConnection(conn.get());
    return true;
}

bool Host::WriteConnection(void) {
    bool res = messagesOut.PopConnection(conn.get());
    sem_post(clientSem);
    return res;
}

void Host::CloseConnection(void) {
    conn->Close();
    sem_close(hostSem);
    sem_close(clientSem);
}
