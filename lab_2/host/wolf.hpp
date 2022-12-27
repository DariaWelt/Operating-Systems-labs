#pragma once
#include <cstdlib>
#include <random>

class Wolf
{
public:
    size_t roll() { return dist(rng); }

private:
    const size_t MIN = 1;
    const size_t MAX = 100;

    std::uniform_int_distribution<std::mt19937::result_type> dist = std::uniform_int_distribution<std::mt19937::result_type>(MIN, MAX);
    std::random_device rd;
    std::mt19937 rng = std::mt19937(rd());
};