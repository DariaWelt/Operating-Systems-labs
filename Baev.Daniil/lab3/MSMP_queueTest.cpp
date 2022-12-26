#include "MSMP_queueTest.h"

#include <pthread.h>
#include <sys/sysinfo.h>
#include <vector>
#include <iostream>
#include <chrono>
//#define CPP11_ATOMIC //to use std::atomic in test
#define CPP11  //to use built-in functions for Memory Model Aware Atomic Operations
#ifdef CPP11_ATOMIC
    #include <atomic>
    #undef CPP11
#endif

struct MSMPQueueProduserArgs{
    std::shared_ptr<MSMP_queue<int>> queue;
    int startNum;
    int endNum;
};

struct MSMPQueueConsumerArgs{
    std::shared_ptr<MSMP_queue<int>> queue;
    #ifdef CPP11_ATOMIC
        std::vector<std::atomic<int>>& nums;
    #else
        std::vector<int>& nums;
    #endif
    int numberForOneTest;
};

static void* MSMPQueueProduser(void* args){
    MSMPQueueProduserArgs* params = (MSMPQueueProduserArgs*)args;
    for (int i = params->startNum; i <= params->endNum; ++i)
        params->queue->push(i);
    return nullptr;
}

static void* MSMPQueueConsumer(void* args){
    MSMPQueueConsumerArgs* params = (MSMPQueueConsumerArgs*)args;
    int readNum = 0;
    while(readNum < params->numberForOneTest){
        std::shared_ptr<int> num = params->queue->pop();
        if (num != nullptr){
            #ifdef CPP11_ATOMIC
                params->nums[*num]++;
            #else
            #ifdef CPP11
                __atomic_add_fetch(&params->nums[*num], 1, __ATOMIC_RELAXED);
            #else
                __sync_fetch_and_add(&params->nums[*num], 1);
            #endif
            #endif
            readNum++;
        }   
    }
    return nullptr;
}

bool MSMPQueuePushTest(size_t threadNum, int numberForOneThread, int repeatNum){
    auto msmpQueue = std::make_shared<MSMP_queue<int>>();
    std::vector<MSMPQueueProduserArgs*> produsersArgs;
    std::vector<pthread_t> thProdusers;

    thProdusers.resize(threadNum * repeatNum);
    produsersArgs.resize(threadNum * repeatNum);

    auto start = std::chrono::steady_clock::now();
    for (int num = 0; num < repeatNum; ++num){
        for (size_t i = 0; i < threadNum; ++i){
            produsersArgs[num * threadNum + i] = new MSMPQueueProduserArgs{msmpQueue, (int)(i * numberForOneThread), (int)((i + 1) * numberForOneThread - 1)};
            int res = pthread_create(&thProdusers[num * threadNum + i], NULL, MSMPQueueProduser, (void*)produsersArgs[num * threadNum + i]);
            if (res != 0)
                std::cout << "ERROR: thread create failed. Number: " << num * threadNum + i << std::endl;
        }
        for (size_t i = 0; i < threadNum; ++i){
            int res = pthread_join(thProdusers[num * threadNum + i], nullptr);
            if (res != 0)
                std::cout << "ERROR: thread join failed " << num * threadNum + i << std::endl;
            }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Test time: " << elapsed_seconds.count() << std::endl;
    std::cout << "Mean test time: " << elapsed_seconds.count() / repeatNum << std::endl;

    for (size_t i = 0; i < threadNum; ++i){
        delete produsersArgs[i];
    }

    std::vector<int> numInQueue;
    numInQueue.resize(threadNum * numberForOneThread);
    for (size_t i = 0; i < threadNum * numberForOneThread * repeatNum; ++i){
        int num;
        if (msmpQueue->empty()){
            std::cout << "FAILED TEST: not enougth number in queue " << i << std::endl;
            return false;
        }
        num = *msmpQueue->pop();
        numInQueue[num]++;
    }

    if (!msmpQueue->empty()){
        std::cout << "FAILED TEST: to many number in queue" << std::endl;
        return false;
    }
    for (size_t i = 0; i < threadNum * numberForOneThread; ++i)
        if(numInQueue[i] != repeatNum){
            std::cout << "FAILED TEST: to many repeat " << i << " in queue" << std::endl;
            return false;
        }

    return true;
}

bool MSMPQueuePopTest(size_t threadNum, int numberForOneThread, int repeatNum){
    auto msmpQueue = std::make_shared<MSMP_queue<int>>();
    std::vector<MSMPQueueConsumerArgs*> consumersArgs;
    std::vector<pthread_t> thConsumers;
    #ifdef CPP11_ATOMIC
        std::vector<std::atomic<int>> checkingVec(numberForOneThread * threadNum * repeatNum);
    #else
        std::vector<int> checkingVec(numberForOneThread * threadNum * repeatNum);
    #endif
    thConsumers.resize(threadNum * repeatNum);
    consumersArgs.resize(threadNum * repeatNum);

    for (size_t i = 0; i < numberForOneThread * threadNum * repeatNum; ++i){
        checkingVec[i] = 0;
    }

    for (size_t i = 0; i < numberForOneThread * threadNum * repeatNum; ++i){
        msmpQueue->push(i);
    }

    auto start = std::chrono::steady_clock::now();
    for (int num = 0; num < repeatNum; ++num){
        for (size_t i = 0; i < threadNum; ++i){
            consumersArgs[num * threadNum + i] = new MSMPQueueConsumerArgs{msmpQueue, checkingVec, numberForOneThread};
            int res = pthread_create(&thConsumers[num * threadNum + i], NULL, MSMPQueueConsumer, (void*)consumersArgs[num * threadNum + i]);
            if (res != 0)
                std::cout << "ERROR: thread create failed. Number: " << num * threadNum + i << std::endl;
        }
        for (size_t i = 0; i < threadNum; ++i){
            int res = pthread_join(thConsumers[num * threadNum + i], nullptr);
            if (res != 0)
                std::cout << "ERROR: thread join failed " << num * threadNum + i << std::endl;
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Test time: " << elapsed_seconds.count() << std::endl;
    std::cout << "Mean test time: " << elapsed_seconds.count() / repeatNum << std::endl;

    for (size_t i = 0; i < threadNum * repeatNum; ++i){
        delete consumersArgs[i];
    }
    

    if (!msmpQueue->empty()){
        std::cout << "FAILED TEST: to many number in queue" << std::endl;
        return false;
    }
    for (size_t i = 0; i < numberForOneThread * threadNum; ++i)
        #ifdef CPP11_ATOMIC
            if(checkingVec[i].load() != 1){
                std::cout << "FAILED TEST: to many repeat " << i << " in queue" << std::endl;
                return false;
            }
        #else
            if(checkingVec[i] != 1){
                std::cout << "FAILED TEST: to many repeat " << i << " in queue" << std::endl;
                return false;
            }
        #endif

    return true;
}

bool MSMPQueuePushPopTest(int number, int repeatNum){
    int maxThreadNum = get_nprocs_conf();
    std::cout << maxThreadNum << std::endl;
    int produserThreadNum;
    int consumerThreadNum;
   for (produserThreadNum = 1; produserThreadNum < maxThreadNum; ++produserThreadNum){
        for (consumerThreadNum = 1; consumerThreadNum + produserThreadNum <= maxThreadNum; ++consumerThreadNum){
            std::cout << produserThreadNum << ' ' << consumerThreadNum << std::endl;

            auto msmpQueue = std::make_shared<MSMP_queue<int>>();
            std::vector<MSMPQueueProduserArgs*> produsersArgs(produserThreadNum * repeatNum);
            std::vector<pthread_t> thProdusers(produserThreadNum * repeatNum);
            std::vector<MSMPQueueConsumerArgs*> consumersArgs(consumerThreadNum * repeatNum);
            std::vector<pthread_t> thConsumers(consumerThreadNum * repeatNum);
            #ifdef CPP11_ATOMIC
                std::vector<std::atomic<int>> checkingVec(numberForOneThread * threadNum * repeatNum);
            #else
                std::vector<int> checkingVec(number);
            #endif

            for (int i = 0; i < number; ++i){
                checkingVec[i] = 0;
            }

            auto start = std::chrono::steady_clock::now();
            for (int num = 0; num < repeatNum; ++num){
                for (int i = 0; i < produserThreadNum; ++i){
                    int startNum = (int)(i * (number / produserThreadNum));
                    int endNum;
                    if (i == produserThreadNum - 1)
                        endNum = number - 1;
                    else 
                        endNum = (int)((i + 1) * (number / produserThreadNum) - 1);
                    produsersArgs[num * produserThreadNum + i] = new MSMPQueueProduserArgs{msmpQueue, startNum, endNum};
                    int res = pthread_create(&thProdusers[num * produserThreadNum + i], NULL, MSMPQueueProduser, (void*)produsersArgs[num * produserThreadNum + i]);
                    if (res != 0)
                        std::cout << "ERROR: thread create failed. Number: " << num * produserThreadNum + i << std::endl;
                }
                for (int i = 0; i < consumerThreadNum; ++i){
                    int numberForOneThread;
                    if (i == produserThreadNum - 1)
                        numberForOneThread = number - number / consumerThreadNum * (consumerThreadNum - 1);
                    else
                        numberForOneThread = number / consumerThreadNum;
                    consumersArgs[num * consumerThreadNum + i] = new MSMPQueueConsumerArgs{msmpQueue, checkingVec, numberForOneThread};
                    int res = pthread_create(&thConsumers[num * consumerThreadNum + i], NULL, MSMPQueueConsumer, (void*)consumersArgs[num * consumerThreadNum + i]);
                    if (res != 0)
                        std::cout << "ERROR: thread create failed. Number: " << num * consumerThreadNum + i << std::endl;
                }

                for (int i = 0; i < produserThreadNum; ++i){
                    int res = pthread_join(thProdusers[num * produserThreadNum + i], nullptr);
                if (res != 0)
                    std::cout << "ERROR: thread join failed " << num * produserThreadNum + i << std::endl;
                }
                for (int i = 0; i < consumerThreadNum; ++i){
                    int res = pthread_join(thConsumers[num * consumerThreadNum + i], nullptr);
                    if (res != 0)
                    std::cout << "ERROR: thread join failed " << num * consumerThreadNum + i << std::endl;
                }
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Test time: " << elapsed_seconds.count() << std::endl;
            std::cout << "Mean test time: " << elapsed_seconds.count() / repeatNum << std::endl;

            for (int i = 0; i < produserThreadNum; ++i){
                delete produsersArgs[i];
            }
            for (int i = 0; i < consumerThreadNum * repeatNum; ++i){
                delete consumersArgs[i];
            }

            if (!msmpQueue->empty()){
                std::cout << "FAILED TEST: to many number in queue" << std::endl;
                return false;
            }
            for (int i = 0; i < number; ++i)
            #ifdef CPP11_ATOMIC
                if(checkingVec[i].load() != repeatNum){
                    std::cout << "FAILED TEST: to many repeat " << i << " in queue" << std::endl;
                    return false;
                }
            #else
                if(checkingVec[i] != repeatNum){
                    std::cout << "FAILED TEST: to many repeat " << i << " in queue" << std::endl;
                    return false;
                }
        #endif
        }
    }
    return true;
}