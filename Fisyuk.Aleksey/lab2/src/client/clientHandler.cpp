#include <semaphore.h>
#include <cstring>
#include "clientHandler.hpp"
#include <iostream>
#include <syslog.h>
#include <chrono>
#include <thread>
#include <unistd.h>

Client &Client::getInstance() {
    static Client instance;
    return instance;
}

Client::Client() {
    struct sigaction act = {0};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handleSignal;
    sigaction(SIGTERM, &act, nullptr);
    sigaction(SIGUSR1, &act, nullptr);
}

void Client::setHostPid(int hostPid) {
    this->_host_pid = hostPid;
}

void Client::openConnection(ConnInfo& info) {
    info.conn = IConn::getConnection();
    syslog(LOG_INFO, "[INFO] connect with pid %i", _host_pid);
    info.conn->openConn(_host_pid, false);

    std::string host_name = "/host_" + std::to_string(_host_pid);
    std::string client_name = "/client_" + std::to_string(_host_pid);
    info._sem_host = sem_open(host_name.c_str(), 0);
    info._sem_client = sem_open(client_name.c_str(), 0);

    if (info._sem_host == SEM_FAILED || info._sem_client == SEM_FAILED) {
        throw std::runtime_error("sem_open failed with error " + std::string(strerror(errno)));
    } else {
        kill(_host_pid, SIGUSR1);
    }
}

void Client::terminate() {
    kill(_host_pid, SIGUSR2);
    info._sem_host = SEM_FAILED;
    info._sem_client = SEM_FAILED;
    info.conn->closeConn();
}

bool Client::sendMessages(ConnInfo& info){
    bool rc = _outputMsg.sendToConn(info.conn);
    sem_post(info._sem_host);
    return rc;
}

void Client::guiSend(IConn::Message msg) {
    Client::getInstance()._outputMsg.push(msg);
}

bool Client::getMessages(ConnInfo& info){
    {
        timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += 10;

        int s = sem_timedwait(info._sem_client, &time);
        if (s == -1)
        {
            syslog(LOG_ERR, "Read timeout");
            _isRunning = false;
            return false;
        }
    }
    _inputMsg.getFromConn(info.conn);
    return true;
}

bool Client::guiGet(IConn::Message *msg) {
    return Client::getInstance()._inputMsg.pop(msg);
}

void Client::processConn(){
    try {
        openConnection(info);

    } catch (std::runtime_error &error) {
        syslog(LOG_ERR, "%s", error.what());
        closelog();
        return;
    }
    _isRunning = true;
    syslog(LOG_INFO, "[INFO] client attached");
    while (_isRunning) {
        if(!sendMessages(info))
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        if(!getMessages(info))
            break;
    }
}

void Client::run() {
    std::thread connectionThread(&Client::processConn, this);
    _gui = new GUI("Client", guiSend, guiGet, isRunning);
    _gui->start();
    
    delete _gui;
    _isRunning = false;
    connectionThread.join();
}

void Client::handleSignal(int signum, siginfo_t *info, void *ptr) {
    switch (signum) {
        case SIGUSR1:
        {
        syslog(LOG_INFO, "[INFO] Host are ready");
        Client::getInstance()._isRunning = true;
        break;
        }
        case SIGTERM:
        case SIGINT:
            syslog(LOG_INFO, "[INFO] stop work");
            Client::getInstance()._isRunning = false;
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}