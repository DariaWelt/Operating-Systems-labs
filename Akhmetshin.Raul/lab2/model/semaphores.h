#pragma once

#include <game.h>

#include <string>
#include <semaphore.h>

struct Semaphores {
public:
    static std::string const host_semaphore_name;
    static std::string const client_semaphore_name;

    struct Message {
        bool goat_status;
        size_t goat_number;
    };

    static bool time_out(struct timespec* ts) {
        int rc = clock_gettime(CLOCK_REALTIME, ts);
        
        if (rc == -1) {
            ts = nullptr;
            return false;
        }

        ts->tv_sec += Game::time_out_sec;
        return true;
    }
};

std::string const Semaphores::host_semaphore_name = "HostSemaphore";
std::string const Semaphores::client_semaphore_name = "WolfSemaphore";
