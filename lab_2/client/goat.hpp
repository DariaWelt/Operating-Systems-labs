#pragma once
#include <cstdlib>
#include <random>

class Goat
{
public:
    size_t roll() { return isAlive ? distAlive(rng) : distDead(rng); }
    void setStatus(bool isAlive) { this->isAlive = isAlive; }
    bool getStatus() const { return isAlive; }

private:
    bool isAlive = true;
    const size_t MIN = 1;
    const size_t ALIVE_MAX = 100;
    const size_t DEAD_MAX = 50;

    std::uniform_int_distribution<std::mt19937::result_type> distAlive = std::uniform_int_distribution<std::mt19937::result_type>(MIN, ALIVE_MAX);
    std::uniform_int_distribution<std::mt19937::result_type> distDead = std::uniform_int_distribution<std::mt19937::result_type>(MIN, DEAD_MAX);
    std::random_device rd;
    std::mt19937 rng = std::mt19937(rd());
};