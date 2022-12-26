#include "set/fine_set.h"
#include "set/opt_set.h"

#include <iostream>
#include <cstdio>

int comp(const int& a, const int& b) {
    return a - b;
}

int main(int argc, char *argv[]) {
    int res;
    OptSet<int> opt_set(res, comp);
    FineSet<int> fine_set(res, comp);

    for (int i = 0; i < 10; i++) {
        fine_set.add(i);
        if (!std::cout << opt_set.contains(i))
            std::cout << "bad_add";
    }

    for (int i = 0; i < 10; i++) {
        fine_set.remove(i);
        if (!std::cout << opt_set.contains(i))
            std::cout << "bad_remove";
    }
    return 0;
}
