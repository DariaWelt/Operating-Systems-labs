#include "wolf.h"
#include "game.h"
#include "semaphores.h"
#include <cstring>
#include <iostream>
#include <random>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


std::unique_ptr<Wolf> Wolf::wolf = nullptr;

Wolf::Wolf() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    openlog(NULL, LOG_PID, 0);
}

Wolf::~Wolf() {
    for (pid_thread_t::iterator iter = goats.begin(); iter != goats.end(); ++iter) {
        sem_t* host_sem = sem_open((Semaphores::host_semaphore_name + std::to_string(iter->first)).c_str(), 0);
        sem_t* client_sem = sem_open((Semaphores::client_semaphore_name + std::to_string(iter->first)).c_str(), 0);

        if (sem_close(host_sem) == -1) {
            syslog(LOG_ERR, "Can't close host semaphore for client with pid %i", iter->first);
        }
        if (sem_close(client_sem) == -1) {
            syslog(LOG_ERR, "Can't close client semaphore for client with pid %i", iter->first);
        }

        syslog(LOG_INFO, "Delete sems");
    }

    for (auto& conn : connections) {
        delete conn;
    }

    closelog();
}

Wolf& Wolf::instanse() {
    if (wolf == nullptr) {
        wolf = std::make_unique<Wolf>();
    }

    return *wolf;
}

void Wolf::signal_handler(int signal_id, siginfo_t* info, void* ptr) {
    auto& inst = instanse();

    pid_t client_pid = info->si_pid;

    switch (signal_id) {
    case SIGUSR1: {
        syslog(LOG_INFO, "Reseive signal to connect from process with pid: %i", client_pid);

        if (inst.goats.count(client_pid) > 0) {
            std::cout << "Process with such pid is exist";
            syslog(LOG_INFO, "Process with such pis already connected");
            break;
        }

        sem_t* host_sem = sem_open((Semaphores::host_semaphore_name + std::to_string(client_pid)).c_str(), O_CREAT | O_EXCL, 0666, 0);
        if (host_sem == SEM_FAILED) {
            syslog(LOG_ERR, "Host semaphore for this client is exist");
            break;
        }
        sem_t* client_sem = sem_open((Semaphores::client_semaphore_name + std::to_string(client_pid)).c_str(), O_CREAT | O_EXCL, 0666, 0);
        if (client_sem == SEM_FAILED) {
            syslog(LOG_ERR, "Client semaphore for this client is exist");
            break;
        }

        syslog(LOG_INFO, "Semaphores for client with pid %i was successfully opened", client_pid);
        syslog(LOG_INFO, "Process with pid %i successfully connected", client_pid);

        inst.goats[client_pid] = 0;

        inst.connections.push_back(Connection::create(client_pid, true, sizeof(Semaphores::Message)));

        kill(client_pid, SIGUSR1);
        if (inst.goats.size() == inst.max_goats_num)
            inst.run();
        break;
        }
    case SIGUSR2:{
        inst.goats.erase(client_pid);
        break;
        }
    case SIGINT:
    case SIGTERM: {
        inst.continue_the_game = false;
        free(inst.wnd);
        free(inst.q_app);
        break;
        }
    }

    return;
}

void* Wolf::game(void* argv) {
    pid_t pid = *(pid_t*)argv;
    std::string pid_as_string = std::to_string(pid);

    bool* goat_status;

    sem_t* host_sem = sem_open((Semaphores::host_semaphore_name + pid_as_string).c_str(), 0);
    if (host_sem == SEM_FAILED) {
        syslog(LOG_ERR, "Cleint with pid %i can not get access to host semaphore", pid);
        return nullptr;
    }

    sem_t* client_sem = sem_open((Semaphores::client_semaphore_name + pid_as_string).c_str(), 0);
    if (client_sem == SEM_FAILED) {
        syslog(LOG_ERR, "Client with pid %i can not get access to client semaphore", pid);
        return nullptr;
    }

    Connection* conn = Connection::create(pid, false);

    struct timespec ts;
    if (!Semaphores::time_out(&ts)) {
        syslog(LOG_ERR, "Can not gat current time");
        return nullptr;
    }
    if (sem_timedwait(client_sem, &ts) == -1) {
        syslog(LOG_ERR, "Client semaphore can not wait on host side");
        return nullptr;
    }

    syslog(LOG_INFO, "Host continue to work after client sem wait");

    {
        Semaphores::Message message;

        if (!conn->Read(&message, sizeof(Semaphores::Message))) {
            syslog(LOG_ERR, "Host can not read");
            return nullptr;
        }

        syslog(LOG_INFO, "Host Get status : %i", (int)message.goat_status);
        syslog(LOG_INFO, "Host Get number : %lu", message.goat_number);

        std::cout << "Client pid: " << pid << ", goat number: " << message.goat_number << ", goat status: " <<
                                                                    (message.goat_status ? "alive" : "dead") << std::endl;

        Wolf& wlf = instanse();
        goat_status = new bool(wlf.chase_goat(message.goat_status, message.goat_number));
        message.goat_status = *goat_status;

        std::cout << "Client pid: " << pid << ", new goat status: " <<  (*goat_status ? "alive" : "dead") << std::endl;

        wlf.wnd->setStatus(pid,message.goat_number,*goat_status);

        if (!conn->Write(&message, sizeof(Semaphores::Message))) {
            syslog(LOG_ERR, "Host can not write");
            return nullptr;
        }

    }

    if (sem_post(host_sem) == -1) {
        syslog(LOG_ERR, "Host semaphore can not post on host side");
        return nullptr;
    }

    syslog(LOG_INFO, "Wating client reading");

    if (!Semaphores::time_out(&ts)) {
        syslog(LOG_ERR, "Can not gat current time");
        return nullptr;
    }
    if (sem_timedwait(client_sem, &ts) == -1) {
        syslog(LOG_ERR, "Client semaphore can not wait on host size");
        return nullptr;
    }

    delete conn;

    return goat_status;
}

int Wolf::run() {
    //syslog(LOG_INFO, "Waiting for all clients connection");

    wnd->setGameStat("Runnig...");
    syslog(LOG_INFO, "All cleints connected. Starting the game");

    for (pid_thread_t::iterator iter = goats.begin(); iter != goats.end(); ++iter) {
        kill(iter->first, SIGUSR2);
        wnd->pid_fil(iter->first);
    }
    //q_app->processEvents();
    syslog(LOG_INFO, "Init clients' threads");

    while(continue_the_game) {
        std::cout << "New iteration" << std::endl;

        bool all_goats_dead = true;

        syslog(LOG_INFO, "All threads was successfully created");
        //wolf_number = generate_number();
        wolf_number = wnd->GetNumber();
        wnd->setWolfNumber(wolf_number);
        std::cout << "wolf number: " << wolf_number << std::endl;


        for (pid_thread_t::iterator iter = goats.begin(); iter != goats.end(); ++iter) {
            if (pthread_create(&(iter->second), nullptr, game, (void*)(&(iter->first))) != 0) {
                syslog(LOG_ERR, "Can not create thread");
                return -1;
            }
        }

        int alive_count=0;

        for (pid_thread_t::iterator iter = goats.begin(); iter != goats.end(); ++iter) {
            void* res;

            if (pthread_join(iter->second, &res) != 0) {
                syslog(LOG_ERR, "Error in thread");
                return -1;
            }
            
            bool is_goat_alive = *(bool*)res;
            all_goats_dead = all_goats_dead && !is_goat_alive;

            alive_count+= is_goat_alive? 1:0;
            delete (bool*)res;
        }

        wnd->setAliveCount(alive_count);
        define_game_status(all_goats_dead);

        next_iter();
    }

    syslog(LOG_INFO, "Finish the game");
    std::cout<<"Finish";
    return 0;
}

void Wolf::set_max_goats_num(size_t goats_num) {
    max_goats_num = goats_num;
}

size_t Wolf::generate_number() const {
    size_t max = Game::max_number_for_wolf;
    size_t min = Game::min_number_for_wolf;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);

    return dist(rng);
}

bool Wolf::chase_goat(bool is_alive, size_t goat_num) const {
    size_t dif;
    dif = std::abs(int(wolf_number - goat_num));

    return is_alive ?
    dif <= Game::alive_goat_dividend / max_goats_num :
    dif <= Game::dead_goat_dividend / max_goats_num;
}

void Wolf::define_game_status(bool is_all_goats_dead) {
    if (is_all_goats_dead) {
        ++cur_turns_counter_without_alive_goats;
        syslog(LOG_INFO, "All goats dead");
    }
    else {
        cur_turns_counter_without_alive_goats = 0;
    }

    if (cur_turns_counter_without_alive_goats == Game::steps_for_end_game) {
        syslog(LOG_INFO, "All goat dead too long");
        continue_the_game = false;
    }
}

void Wolf::next_iter() {
    for (pid_thread_t::iterator iter = goats.begin(); iter != goats.end(); ++iter) {
        if (!continue_the_game) {
            kill(iter->first, SIGTERM);
            wnd->setGameStat("Game over!");
        }

        sem_t* host_sem = sem_open((Semaphores::host_semaphore_name + std::to_string(iter->first)).c_str(), 0);

        if (sem_post(host_sem) == -1) {
            syslog(LOG_ERR, "Can not post host semaphore");
        }
    }
}
