#include "goat.h"

Goat Goat::instance;

Goat& Goat::getInstance() {

    return Goat::instance;
}

Goat::~Goat() {

    if (_semHost != SEM_FAILED) {

        sem_close(_semHost);
    }

    if (_semClient != SEM_FAILED) {

        sem_close(_semClient);
    }

    if (!_conn->close()) {

        exit(errno);
    }

    syslog(LOG_INFO, "Goat was dead");
    std::cout << "Goat was dead" << std::endl;
    closelog();
}

void Goat::prepareGame() {

    _conn = Conn::createConnection();

    if (_hostPid == -1) {

        throw std::runtime_error("Host pid isn't setted");
    }

    if (!_conn->open(_hostPid, false)) {

        exit(errno);
    }

    openlog("GameClient", LOG_PID, LOG_DAEMON);

    std::string semHostName = std::string("host_" + std::to_string(_hostPid));
    std::string semClientName = std::string("client_" + std::to_string(_hostPid));

    _semHost = sem_open(semHostName.c_str(), 0);

    if (_semHost == SEM_FAILED) {

        throw std::runtime_error("sem_host wasn't opened");
    }

    _semClient = sem_open(semClientName.c_str(), 0);

    if (_semClient == SEM_FAILED) {

        sem_close(_semHost);
        throw std::runtime_error("sem_client wasn't opened");
    }

    signal(SIGUSR1, onSignalReceive);
    signal(SIGTERM, onSignalReceive);
    signal(SIGINT, onSignalReceive);

    kill(_hostPid, SIGUSR1);
}

void Goat::startGame() {

    std::cout << "Goat's started hiding..." << std::endl;
    std::cout << "___________GAME_STEP___________" << std::endl;

    if (!generateAndWriteValue()) {
        return;
    }

    sem_post(_semHost);

    while (true) {

        std::cout << "___________GAME_STEP___________" << std::endl;

        if (!waitForHost(_semClient)) {

            return;
        }

        Conn::Msg msg;
        memset(&msg, 0, sizeof(msg));

        if (!_conn->read(&msg, sizeof(msg))) {

            return;
        }

        _goatState = static_cast<GOAT_STATE>(msg.data);

        if (!generateAndWriteValue()) {

            return;
        }

        sem_post(_semHost);
    }
}

int Goat::generateValue() {

    std::random_device rd;
    std::mt19937 gen(rd());

    if (isAlive()) {

        std::uniform_int_distribution<int> dis(GAME_CONSTANTS::MIN_GOAT_VAL_ALIVE, GAME_CONSTANTS::MAX_GOAT_VAL_ALIVE);
        return dis(gen);
    } else {

        std::uniform_int_distribution<int> dis(GAME_CONSTANTS::MIN_GOAT_VAL_DEAD, GAME_CONSTANTS::MAX_GOAT_VAL_DEAD);
        return dis(gen);
    }
}

bool Goat::generateAndWriteValue() {

    Conn::Msg msg;
    int cur_val = generateValue();
    std::cout << "Goat: My number is " << cur_val << std::endl;

    msg.type = Conn::MSG_TYPE::TO_WOLF;
    msg.data = cur_val;

    if (!_conn->write(&msg, sizeof(msg))) {

        return false;
    }
    return true;
}

bool Goat::waitForHost(sem_t* sem) {

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += GAME_CONSTANTS::TIMEOUT;

    if (sem_timedwait(sem, &ts) == -1) {
        syslog(LOG_ERR, "Semaphore timeout in client");
        return false;
    }

    return true;
}

void Goat::onSignalReceive(int sig) {

    Goat& goat = getInstance();

    switch (sig) {
        case SIGTERM:
        case SIGINT:
            syslog(LOG_INFO, "Terminate client (signal)");
            std::cout << "Terminate client (signal)" << std::endl;
            exit(EXIT_SUCCESS);
        case SIGUSR1:
            kill(goat._hostPid, SIGTERM);
            exit(EXIT_SUCCESS);
        default:
            syslog(LOG_INFO, "Unknown signal recieve: continue work");
            std::cout << "Unknown signal recieve: continue work" << std::endl;
            break;
    }
}