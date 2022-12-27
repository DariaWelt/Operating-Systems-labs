#pragma once

#include "lockFreeQueue.hpp"

bool lockFreeQueuePushTest(size_t threadNum, int numberForOneThread, int repeatNum);

bool lockFreeQueuePopTest(size_t threadNum, int numberForOneThread, int repeatNum);

bool lockFreeQueuePushPopTest(int number, int repeatNum);