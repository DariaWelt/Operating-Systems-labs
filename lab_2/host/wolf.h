#ifndef LAB2_WOLF_H
#define LAB2_WOLF_H

#include "../conn/conn.h"
#include "../settings/GameSettings.h"
#include <filesystem>


class Wolf {
public:
    ~Wolf();

    static Wolf& getInstance();

    void setHostPid(int hostPid) { _hostPid = hostPid; }

    void startGame();
    void prepareGame();

private:
    Wolf() {};

    Wolf(Wolf& w) = delete;
    Wolf& operator=(const Wolf& w) = delete;

    int generateValue();
    bool waitForClient(sem_t* sem);
    bool semSignal(sem_t* sem);

    static void handleSignal(int sig, siginfo_t* info, void* ptr);
private:

    static Wolf instance;

    Conn* _conn;
    sem_t* _pSemHost;
    sem_t* _pSemClient;

    GOAT_STATE _goatState = GOAT_STATE::ALIVE;

    bool _isClientConnected;
    int _clientPid;
    int _hostPid = -1;

    std::string _semHostName;
    std::string _semClientName;
};

#endif //LAB2_WOLF_H
