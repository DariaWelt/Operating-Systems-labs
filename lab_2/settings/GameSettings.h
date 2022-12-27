#ifndef LAB2_GAMESETTINGS_H
#define LAB2_GAMESETTINGS_H

#include <mqueue.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <iostream>
#include <csignal>
#include <cstring>
#include <random>
#include <string>
#include <signal.h>
#include <cstdlib>
#include <fcntl.h>


namespace GAME_CONSTANTS {
    const int TIMEOUT = 5;
    const int SEM_RECONNECT_TIMEOUT = 5;
    const int MAX_GOAT_VAL_ALIVE = 100;
    const int MAX_GOAT_VAL_DEAD = 50;
    const int MIN_GOAT_VAL_ALIVE = 1;
    const int MIN_GOAT_VAL_DEAD = 1;
    const int MAX_WOLF_VAL = 100;
    const int MIN_WOLF_VAL = 1;
    const int ALIVE_DEVIATION = 70;
    const int DEAD_DEVIATION = 20;
}

enum class GOAT_STATE
{
    ALIVE = 0,
    HALF_DEAD,
    DEAD
};

#endif //LAB2_GAMESETTINGS_H
