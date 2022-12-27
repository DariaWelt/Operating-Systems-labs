#include "hostHandler.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <syslog.h>
#include <chrono>
#include <thread>

Host &Host::getInstance() {
    static Host instance;
    return instance;
}

Host::Host() {

    struct sigaction act = {0};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handleSignal;

    if(sigaction(SIGTERM, &act, nullptr) == -1) {
        throw std::runtime_error("sigaction error " + std::string(strerror(errno)));
    }
    if(sigaction(SIGUSR1, &act, nullptr) == -1) {
        throw std::runtime_error("sigaction error " + std::string(strerror(errno)));
    }
    if(sigaction(SIGUSR2, &act, nullptr) == -1) {
        throw std::runtime_error("sigaction error " + std::string(strerror(errno)));
    }
    if(sigaction(SIGINT, &act, nullptr) == -1) {
        throw std::runtime_error("sigaction error " + std::string(strerror(errno)));
    }
}

void Host::openConn(ConnInfo &info) {
    info.conn = IConn::getConnection();
    syslog(LOG_INFO, "[INFO] open connection with pid %i", getpid());
    if(_isCreatedConn){
        return;
    }
    info.conn->openConn(getpid(), true);

    std::string host_name = "/host_" + std::to_string(getpid());
    std::string client_name = "/client_" + std::to_string(getpid());
    info._sem_host = sem_open(host_name.c_str(), O_CREAT | O_EXCL, 0777, 0);
    info._sem_client = sem_open(client_name.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (info._sem_client == SEM_FAILED || info._sem_host == SEM_FAILED) {
        throw std::runtime_error("sem_open error " + std::string(strerror(errno)));
    }
    _isCreatedConn = true;
}

bool Host::sendMessages(ConnInfo& info){
    bool rc = _outputMsg.sendToConn(info.conn);
    sem_post(info._sem_client);
    return rc;
}

void Host::guiSend(IConn::Message msg) {
    Host::getInstance()._outputMsg.push(msg);
}

bool Host::getMessages(ConnInfo& info){
    {
        timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += 5;

        int s = sem_timedwait(info._sem_host, &time);
        if (s == -1)
        {
            _isRunning = false;
            return false;
        }
    }
    _inputMsg.getFromConn(info.conn);
    return true;
}

bool Host::guiGet(IConn::Message *msg)
{
    return Host::getInstance()._inputMsg.pop(msg);
}

void Host::processConn(){
    while(_isRunning){
        try {
            openConn(info);
        } catch (std::runtime_error &error) {
            syslog(LOG_ERR, "%s", error.what());
            closelog();
            return;
        }

        auto clock = std::chrono::high_resolution_clock::now();
        while (_isRunning.load()) {
            if(_clientPid == -1)
                continue;
            double minutesPassed = std::chrono::duration_cast<std::chrono::minutes>(
                std::chrono::high_resolution_clock::now() - clock).count();

            if (minutesPassed >= 1) {
                syslog(LOG_INFO, "Kill client");
                kill(_clientPid, SIGTERM);
                _clientPid = -1;
                break;
            }

            if(!getMessages(info))
                return;

            if(!sendMessages(info))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }
}

void Host::start() {
    std::thread connectionThread(&Host::processConn, this);
    _isRunning = true;
    _gui = new GUI("Host", guiSend, guiGet, isRunning);
    _gui->start();
    
    delete _gui;
    connectionThread.join();
}


void Host::terminate() {
    _isRunning = false;
    info._sem_host = SEM_FAILED;
    info._sem_client = SEM_FAILED;
    info.conn->closeConn();
}

void Host::handleSignal(int signum, siginfo_t *info, void *ptr) {
    static Host& host = getInstance();

    switch (signum) {
        case SIGUSR1:
            if (host._clientPid != -1){
                syslog(LOG_INFO, "[INFO] client has been already connected");
            } else {
                syslog(LOG_INFO, "[INFO] attaching client with pid = %i" ,info->si_pid);
                host._clientPid = info->si_pid;
            }
            break;
        case SIGUSR2:
            syslog(LOG_INFO, "[INFO] disconnect client");
            if(host._clientPid == info->si_pid){
                host._clientPid = -1;
            }
            break;
        case SIGTERM:
        case SIGINT:
            if (host._clientPid != -1) {
                kill(host._clientPid, signum);
                host._clientPid = -1;
            }
            host.terminate();
            std::exit(EXIT_SUCCESS);
            break;
        default:
            syslog(LOG_INFO, "[INFO] unknown signal");
            break;
    }
}
