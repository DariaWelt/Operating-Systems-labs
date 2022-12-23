#pragma once

struct Game {
public:
    static size_t const max_number_for_wolf = 100;
    static size_t const min_number_for_wolf = 1;

    static size_t const max_number_for_alve_goat = 100;
    static size_t const max_number_for_dead_goat = 50;
    static size_t const min_number_for_goat = 1;

    static size_t const alive_goat_dividend = 70;
    static size_t const dead_goat_dividend = 20; 

    static size_t const steps_for_end_game = 2;

    static size_t const time_out_sec = 5;
};
