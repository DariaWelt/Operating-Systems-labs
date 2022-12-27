#include "wolf.h"
#include <semaphore.h>

Wolf Wolf::instance;

Wolf& Wolf::getInstance() {

    return Wolf::instance;
}

Wolf::~Wolf() {
    if (_isClientConnected) {

        kill(_clientPid, SIGTERM);
    }

    if (_pSemHost != SEM_FAILED) {

        sem_unlink(_semHostName.c_str());
    }

    if (_pSemClient != SEM_FAILED) {

        sem_unlink(_semClientName.c_str());
    }

    if (!_conn->close()) {

        exit(errno);
    }

    syslog(LOG_INFO, "Wolf has won the game");
    std::cout << "Wolf has won the game" << std::endl;
    closelog();
}


void Wolf::prepareGame() {

    _conn = Conn::createConnection();

    if (_hostPid == -1) {

        throw std::runtime_error("Host pid isn't setted");
    }

    if (!_conn->open(_hostPid, true)) {

        exit(errno);
    }

    openlog("GameHost", LOG_PID, LOG_DAEMON);

    _semHostName = std::string("host_" + std::to_string(_hostPid));
    _semClientName = std::string("client_" + std::to_string(_hostPid));

    _isClientConnected = false;
    _clientPid = 0;

    _pSemHost = sem_open(_semHostName.c_str(), O_CREAT, 0666, 0);
    if (_pSemHost == SEM_FAILED) {

        throw std::runtime_error("sem_host wasn't opened");
    }

    _pSemClient = sem_open(_semClientName.c_str(), O_CREAT, 0666, 0);
    if (_pSemClient == SEM_FAILED) {

        sem_unlink(std::string("host_" + std::to_string(_hostPid)).c_str());
        throw std::runtime_error("sem_client wasn't opened");
    }

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handleSignal;

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGUSR2, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
    pause();
}


void Wolf::startGame() {

    syslog(LOG_INFO, "Wolf's started hunting...");
    std::cout << "Wolf's started hunting..." << std::endl;

    while (_goatState != GOAT_STATE::DEAD) {

        Conn::Msg msg;

        if (!waitForClient(_pSemHost)) {
            return;
        }

        std::cout << "___________GAME_STEP___________" << std::endl;

        if (!_conn->read(&msg, sizeof(msg))) {
            return;
        }

        int goatVal = msg.data;
        std::cout << "Goat: My num is: " << goatVal << std::endl;

        int curVal = generateValue();
        std::cout << "Wolf: My num is " << curVal << std::endl;

        if (_goatState == GOAT_STATE::ALIVE && abs(goatVal - curVal) > GAME_CONSTANTS::ALIVE_DEVIATION) {

            _goatState = GOAT_STATE::HALF_DEAD;
        }
        else if (_goatState == GOAT_STATE::HALF_DEAD) {

            if (abs(goatVal - curVal) > GAME_CONSTANTS::DEAD_DEVIATION) {

                _goatState = GOAT_STATE::DEAD;
            }
            else {

                _goatState = GOAT_STATE::ALIVE;
            }
        }

        switch (_goatState) {

            case GOAT_STATE::ALIVE:
                std::cout << "Goat: I am still alive" << std::endl;
                break;

            case GOAT_STATE::HALF_DEAD:
                std::cout << "Goat: Oh, I got hurt, but I still have a chance to win" << std::endl;
                break;

            case GOAT_STATE::DEAD:
                std::cout << "Goat: Ops, I'm a loser :(" << std::endl;
                break;
        }

        msg.type = Conn::MSG_TYPE::TO_GOAT;
        msg.data = static_cast<long>(_goatState);
        if (!_conn->write(&msg, sizeof(msg))) {

            return;
        }

        sem_post(_pSemClient);
    }
}


int Wolf::generateValue() {

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(GAME_CONSTANTS::MIN_WOLF_VAL, GAME_CONSTANTS::MAX_WOLF_VAL);
    return uni(rng);
}


bool Wolf::waitForClient(sem_t* sem) {

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += GAME_CONSTANTS::TIMEOUT;

    if (sem_timedwait(sem, &ts) == -1) {

        syslog(LOG_ERR, "Semaphore timeout");
        return false;
    }

    return true;
}


void Wolf::handleSignal(int sig, siginfo_t* info, void* ptr) {

    static Wolf& wolf = getInstance();
    switch (sig) {

        case SIGUSR1:
            if (wolf._isClientConnected) {
                syslog(LOG_INFO, "Ignore handshake, client has already connected");
                std::cout << "Ignore handshake, client has already connected" << std::endl;
                kill(info->si_pid, SIGTERM);
            } else {
                syslog(LOG_INFO, "Client connected: pid is %d", info->si_pid);
                std::cout << "Client connected: pid is " << info->si_pid << std::endl;
                wolf._isClientConnected = true;
                wolf._clientPid = info->si_pid;
            }
            break;

        case SIGUSR2:
            syslog(LOG_INFO, "The error has occured on client side");
            std::cout << "The error has occured on client side" << std::endl;
            wolf._isClientConnected = false;
            wolf._clientPid = 0;
            break;

        case SIGTERM:
        case SIGINT:
            syslog(LOG_INFO, "Host is terminated");
            std::cout << "Host is terminated" << std::endl;
            exit(EXIT_SUCCESS);

        default:
            syslog(LOG_INFO, "Unknown signal: continue work");
            std::cout << "Unknown signal: continue work" << std::endl;
            break;
    }
}
