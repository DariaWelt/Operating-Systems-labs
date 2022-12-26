#include "client.h"

#include <sys/syslog.h>
#include <csignal>
#include <cstring>
#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    openlog("Chat client", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);
    
    if (argc != 2) {
        syslog(LOG_ERR, "Host pid was expected as the command argument");
        closelog();
        return EXIT_FAILURE;
    }
    
    int pid;
    try {
        pid = std::atoi(argv[1]);
    }
    catch (std::exception& err) {
        syslog(LOG_ERR, "Could not read host pid");
        closelog();
        return EXIT_FAILURE;
    }

    int returnCode = Client::GetInstance().Start(pid);

    closelog();
    return returnCode;
}

Client::Client(void) {
    struct sigaction sig{};
    std::memset(&sig, 0, sizeof(sig));
    sig.sa_sigaction = SignalHandler;
    sig.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGUSR1, &sig, nullptr);
}

// Function to handle signals
void Client::SignalHandler(int signum, siginfo_t* info, void* ptr) {
    switch (signum) {
        case SIGUSR1:
            syslog(LOG_INFO, "Client got signal from host");
            Client::GetInstance().isConnected = true;
            break;
        case SIGTERM:
            Client::GetInstance().Stop();
            break;
    }
}

// Function to start client
int Client::Start(pid_t hostPid) {
    syslog(LOG_INFO, "Client started");
    isAlive = true;
    this->hostPid = hostPid;

    if (kill(hostPid, SIGUSR1) != 0) {
        isAlive = false;
        syslog(LOG_INFO, "Could not connect to host");
        return EXIT_FAILURE;
    }
    syslog(LOG_INFO, "Send signal to host");

    try {
        std::thread connectionThread(&Client::WorkConnection, this);
        
        gui = new GUI("Client", GUIGet, GUISend, IsAlive);
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

// Function to stop client
void Client::Stop(void) {
    if (isAlive.load()) {
        syslog(LOG_INFO, "Client stopped working");
        isAlive = false;
    }
}

void Client::WorkConnection(void) {
    auto lastTimeCheckClient = std::chrono::high_resolution_clock::now();
    gui->SetConnected(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (!isConnected.load()) {
        double timePassed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - lastTimeCheckClient).count();
        if (timePassed >= _connInterval) {
            isConnected = false;
            hostPid = -1;
            syslog(LOG_ERR, "Host could not prepare in 5 sec, exiting...");
            Stop();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2)); 
    }

    if (!OpenConnection()) {
        return;
    }

    gui->SetConnected(true);
    while (isAlive.load()) {
        if (!WriteConnection()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (!ReadConnection()) {
            break;
        }
    }
    CloseConnection();
    gui->SetConnected(false);
}

bool Client::OpenConnection(void) {
    syslog(LOG_INFO, "Opening connection");
    conn = ConnectionInterface::Create(getpid(), false);

    std::string semNameHost = "/host_" + std::to_string(getpid());
    std::string semNameClient = "/client_" + std::to_string(getpid());
    clientSem = sem_open(semNameClient.c_str(), 0);
    if (clientSem == SEM_FAILED) {
        syslog(LOG_ERR, "Error while creating client semaphore");
        return false;
    }
    hostSem = sem_open(semNameHost.c_str(), 0);
    if (hostSem == SEM_FAILED) {
        sem_close(clientSem);
        syslog(LOG_ERR, "Error while creating client semaphore");
        return false;
    }

    syslog(LOG_INFO, "Semaphores created");

    try {
        conn.get()->Open(hostPid, false);
    }
    catch (std::exception &err) {
        sem_close(hostSem);
        sem_close(clientSem);
        syslog(LOG_ERR, "Error while opening connection: %s", err.what());
        return false;
    }

    syslog(LOG_INFO, "Opened connection");
    return true;
}

bool Client::ReadConnection(void) {
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);

        t.tv_sec += _waitInterval;
        int s = sem_timedwait(clientSem, &t);
        if (s == -1) {
            syslog(LOG_ERR, "Client semaphore timeout");
            isAlive = false;
            return false;
        }
    }
    messagesIn.PushConnection(conn.get());
    return true;
}

bool Client::WriteConnection(void) {
    bool res = messagesOut.PopConnection(conn.get());
    sem_post(hostSem);
    return res;
}

void Client::CloseConnection(void) {
    conn->Close();
    sem_close(hostSem);
    sem_close(clientSem);
}
