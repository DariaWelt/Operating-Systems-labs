#pragma once

#include "MSMP_queue.hpp"

bool MSMPQueuePushTest(size_t threadNum, int numberForOneThread, int repeatNum);

bool MSMPQueuePopTest(size_t threadNum, int number, int repeatNum);

bool MSMPQueuePushPopTest(int number, int repeatNum);