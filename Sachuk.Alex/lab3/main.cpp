#include <iostream>
#include <fstream>
#include <sys/syslog.h>

#include "set/fine_set.h"
#include "set/opt_set.h"

#include "tester/utils.h"
#include "tester/tester.h"

int cmp(const int& a, const int& b) {
    return a - b;
}


void writeTesting(int threadsCnt = 4, int arrSize = 4000) {
    int res = 0;
    Tester<FineSet<int>>::getInstance().init(res, cmp);
    Tester<OptSet<int>>::getInstance().init(res, cmp);

    Tester<FineSet<int>>::getInstance().checkWriteCorrectWork(threadsCnt, arrSize);
    Tester<OptSet<int>>::getInstance().checkWriteCorrectWork(threadsCnt, arrSize);
}

void readTesting(int threadsCnt = 4, int arrSize = 4000) {
    int res = 0;
    Tester<FineSet<int>>::getInstance().init(res, cmp);
    Tester<OptSet<int>>::getInstance().init(res, cmp);

    Tester<FineSet<int>>::getInstance().checkReadCorrectWork(threadsCnt, arrSize);
    Tester<OptSet<int>>::getInstance().checkReadCorrectWork(threadsCnt, arrSize);
}

void generalTesting(int threadsCnt = 4, int arrSize = 4000) {
    int res = 0;
    Tester<FineSet<int>>::getInstance().init(res, cmp);
    Tester<OptSet<int>>::getInstance().init(res, cmp);

    Tester<FineSet<int>>::getInstance().checkGeneralCorrectWork(threadsCnt, arrSize);
    Tester<OptSet<int>>::getInstance().checkGeneralCorrectWork(threadsCnt, arrSize);
}

void performanceTesting(int threadsCnt = 4, int arrSize = 1000, int testsSize = 5) {
    int res = 0;
    Tester<FineSet<int>>::getInstance().init(res, cmp);
    Tester<OptSet<int>>::getInstance().init(res, cmp);

    auto writeItems = generateItems(arrSize, 42);
    Tester<FineSet<int>>::getInstance().checkWritePerformance(threadsCnt, arrSize, testsSize, writeItems);
    Tester<OptSet<int>>::getInstance().checkWritePerformance(threadsCnt, arrSize, testsSize, writeItems);

    auto readItems = generateUniqueItems(arrSize, 42);
    Tester<FineSet<int>>::getInstance().checkReadPerformance(threadsCnt, arrSize, testsSize, readItems);
    Tester<OptSet<int>>::getInstance().checkReadPerformance(threadsCnt, arrSize, testsSize, readItems);

    auto generalItems = generateUniqueItems(arrSize, 42);
    Tester<FineSet<int>>::getInstance().checkGeneralPerformance(threadsCnt, arrSize, testsSize, generalItems);
    Tester<OptSet<int>>::getInstance().checkGeneralPerformance(threadsCnt, arrSize, testsSize, generalItems);
}

int main(int argc, char* argv[]) {
    openlog("Lab3 log", LOG_NDELAY | LOG_PID, LOG_USER);

    writeTesting();
    readTesting();
    generalTesting();

    performanceTesting();

	return 0;
}