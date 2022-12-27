#pragma once

#include <cstdlib>
#include <algorithm>
#include <vector>

int randUniform(int minNum, int maxNum) {
    return int((1.0 * std::rand() + 1) / (1.0 * RAND_MAX + 1) * (maxNum - minNum) + minNum);
}

std::vector<int> generateItems(int size, int seed) {
    constexpr int MIN = 0;
    const int MAX = 3 * size;
        
    std::vector<int> res;
    res.reserve(size);

    std::srand(seed);
    for (int i = 0; i < size; ++i)
        res.push_back(randUniform(MIN, MAX));

    return res;
}

std::vector<int> generateUniqueItems(int size, int seed) {
    std::vector<int> res;
    res.reserve(size);

    for (int i = 0; i < size; ++i)
        res.push_back(i);

    std::random_shuffle(std::begin(res), std::end(res));

    return res;
}
