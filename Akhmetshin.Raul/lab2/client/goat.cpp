#include "goat.h"
#include "game.h"
#include "semaphores.h"

#include <fcntl.h>
#include <cstring>
#include <random>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

std::unique_ptr<Goat> Goat::goat = nullptr;

Goat::Goat() : is_alive(true) {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);

    openlog(NULL, LOG_PID, 0);
}

Goat::~Goat() {
    closelog();
}

Goat& Goat::instanse() {
    if (goat == nullptr) {
        goat = std::make_unique<Goat>();
    }

    return *goat;
}

void Goat::signal_handler(int signal_id, siginfo_t* info, void* ptr) {
    Goat& inst = instanse();

    switch (signal_id) {
    case SIGUSR1:        
        inst.connection_created = true;
        syslog(LOG_INFO, "Host accepted request for connect");
        break;

    case SIGUSR2:
        inst.host_is_ready = true;
        syslog(LOG_INFO, "All clients connected and clients can start work");
        break;

    case SIGINT:
    case SIGTERM:
        inst.continue_the_game = false;
        syslog(LOG_INFO, "Client receive siganl to finish the game");
        //exit(EXIT_SUCCESS);
        inst.end_work();
        break;
    }

    return;
}

int Goat::run() {
    while(!connection_created) {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            syslog(LOG_ERR, "Can not get current time");
            return end_work();
        }

        if (abs(ts.tv_sec - send_request_to_connect_time.tv_sec) > Game::time_out_sec) {
            syslog(LOG_ERR, "Client can not connect to host");
            return end_work();
        }
    }
    while(!host_is_ready);
    pid_t pid = getpid();

    syslog(LOG_INFO, "Client run");

    std::string pid_as_string = std::to_string(pid);
    sem_t* host_sem = sem_open((Semaphores::host_semaphore_name + pid_as_string).c_str(), 0);
    if (host_sem == SEM_FAILED) {
        syslog(LOG_ERR, "Cleint with pid %i can not get access to host semaphore", pid);
        return end_work();
    }

    sem_t* client_sem = sem_open((Semaphores::client_semaphore_name + pid_as_string).c_str(), 0);
    if (client_sem == SEM_FAILED) {
        syslog(LOG_ERR, "Client with pid %i can not get access to client semaphore", pid);
        return end_work();
    }

    while (continue_the_game) {
        Connection* conn = Connection::create(getpid(), false);

        {
            Semaphores::Message message { is_alive, generate_number() };

            if (!conn->Write(&message, sizeof(Semaphores::Message))) {
                syslog(LOG_ERR, "Client with pid %i can not write message", pid);
                std::cout << "err" << std::endl;
                return end_work();
            }

        }

        syslog(LOG_INFO, "Client with pid %i wrote message successfully", pid);
        
        if (sem_post(client_sem) == -1) {
            syslog(LOG_ERR, "Client with pid %i can not post semaphore", pid);
            return end_work();
        }

        struct timespec ts;
        if (!Semaphores::time_out(&ts)) {
            syslog(LOG_ERR, "Can not get current time");
            return end_work();
        }
        if (sem_timedwait(host_sem, &ts) == -1) {
            syslog(LOG_ERR, "Client with pid %i can not wait host semaphore", pid);
            return end_work();
        }

        syslog(LOG_INFO, "Client after host post semaphore");

        {
            Semaphores::Message message;

            if (!conn->Read(&message, sizeof(Semaphores::Message))) {
                syslog(LOG_ERR, "Client can not read host message");
                return end_work();
            }

            syslog(LOG_INFO, "Client read status %i", (int)message.goat_status);

            is_alive = message.goat_status;
        }

        if (sem_post(client_sem) == -1) {
            syslog(LOG_ERR, "Client semaphore can not post");
            return end_work();
        }

        delete conn;

        if (!Semaphores::time_out(&ts)) {
            syslog(LOG_ERR, "Can not get current time");
            return end_work();
        }
        if (sem_timedwait(host_sem, &ts) == -1) {
            syslog(LOG_ERR, "Can not wait host semaphore");
        }

    }

    syslog(LOG_INFO, "Client with pid %i finish the game without problems", pid);

    return 0;
}

int Goat::end_work() {
    kill(host_pid, SIGUSR2);
    return -1;
}

bool Goat::connect_to_host(pid_t pid) {
    syslog(LOG_INFO, "Client want to connect");

    struct stat sts;
    if (stat((std::string("/proc/") + std::to_string(pid)).c_str(), &sts) == -1 && errno == ENOENT) {
        syslog(LOG_ERR, "Process with pid %i does not exist", pid);
        return false;
    }

    host_pid = pid;

    kill(host_pid, SIGUSR1);

    if (clock_gettime(CLOCK_REALTIME, &send_request_to_connect_time) == -1) {
        syslog(LOG_ERR, "Can not get current time");
        return false;
    }

    return true;
}

size_t Goat::generate_number() const {
    size_t max = is_alive ? Game::max_number_for_alve_goat : Game::max_number_for_dead_goat;
    size_t min = Game::min_number_for_goat;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);

    return dist(rng);
}
