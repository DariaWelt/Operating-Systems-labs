#pragma once

#include "conn.h"

#include <memory>
#include <signal.h>

class Goat {
private:
    using goat_ptr = std::unique_ptr<Goat>;

private:
    static goat_ptr goat;

    bool is_alive;

    pid_t host_pid;
    bool connection_created = false;
    bool host_is_ready = false;
    bool continue_the_game = true;

    struct timespec send_request_to_connect_time;

public:
    Goat();
    ~Goat();

    static Goat& instanse();

    int run();
    bool connect_to_host(pid_t pid);

private:
    static void signal_handler(int signal_id, siginfo_t* info, void* ptr);

    size_t generate_number() const;
    int end_work();

};
