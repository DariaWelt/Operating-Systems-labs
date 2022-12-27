#include "lockFreeQueueTest.h"

#include <pthread.h>
#include <sys/sysinfo.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <string>
#define CPP11_ATOMIC //to use std::atomic in test
#define CPP11  //to use built-in functions for Memory Model Aware Atomic Operations
#ifdef CPP11_ATOMIC
    #include <atomic>
    #undef CPP11
#endif

struct LockFreeQueueProduserArgs{
    std::shared_ptr<lock_free_queue<int>> queue;
    int startNum;
    int endNum;
};

struct LockFreeQueueConsumerArgs{
    std::shared_ptr<lock_free_queue<int>> queue;
    #ifdef CPP11_ATOMIC
        std::vector<std::atomic<int>>& nums;
    #else
        std::vector<int>& nums;
    #endif
    int numberForOneTest;
};

static void* LockFreeQueueProduser(void* args){
    LockFreeQueueProduserArgs* params = (LockFreeQueueProduserArgs*)args;
    for (int i = params->startNum; i <= params->endNum;){
        params->queue->push(i);
        //std::cout << "Push " << i << ' ';
        i++;
    }
    //std::cout << std::endl;
    std::string str = "Pushed " + std::to_string(params->endNum - params->startNum + 1) + " number\n";
    std::cout << str;
    return nullptr;
}

static void* LockFreeQueueConsumer(void* args){
    LockFreeQueueConsumerArgs* params = (LockFreeQueueConsumerArgs*)args;
    int readNum = 0;
    int readFailed = 0;
    while(readNum < params->numberForOneTest){
        std::unique_ptr<int> num = params->queue->pop();
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
            readFailed = 0;
        }   
        else{
            readFailed++;
            sched_yield();
            if (readFailed >= 100000){
                std::cout << "Many failed of read\n";
                break;
            }
        }
    }
    std::string str = "Total read " + std::to_string(readNum) + "\n";
    std::cout << str;
    return nullptr;
}

bool lockFreeQueuePushTest(size_t threadNum, int numberForOneThread, int repeatNum){
    auto lockFreeQueue = std::make_shared<lock_free_queue<int>>();
    std::vector<LockFreeQueueProduserArgs*> produsersArgs;
    std::vector<pthread_t> thProdusers;

    thProdusers.resize(threadNum * repeatNum);
    produsersArgs.resize(threadNum * repeatNum);

    auto start = std::chrono::steady_clock::now();
    for (int num = 0; num < repeatNum; ++num){
        for (size_t i = 0; i < threadNum; ++i){
            produsersArgs[num * threadNum + i] = new LockFreeQueueProduserArgs{lockFreeQueue, (int)(i * numberForOneThread), (int)((i + 1) * numberForOneThread - 1)};
            int res = pthread_create(&thProdusers[num * threadNum + i], NULL, LockFreeQueueProduser, (void*)produsersArgs[num * threadNum + i]);
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

    for (size_t i = 0; i < threadNum * repeatNum; ++i){
        delete produsersArgs[i];
    }

    //lockFreeQueue->testQueue();

    std::vector<int> numInQueue;
    numInQueue.resize(threadNum * numberForOneThread);
    for (size_t i = 0; i < threadNum * numberForOneThread * repeatNum; ++i){
        std::unique_ptr<int> num;
        
        if (lockFreeQueue->empty()){
            std::cout << "FAILED TEST: not enougth number in queue" << std::endl;
            return false;
        }
        num = lockFreeQueue->pop();
        numInQueue[*num]++;
    }

    if (!lockFreeQueue->empty()){
        std::cout << "FAILED TEST: to many number in queue" << std::endl;
        return false;
    }

    bool isFailed = false;
    for (size_t i = 0; i < threadNum * numberForOneThread; ++i){
        if(numInQueue[i] != repeatNum){
            std::cout << "FAILED TEST: to many repeat " << i << " in queue" << numInQueue[i] << "numbaer" << std::endl;
            isFailed = true;
        }
    }
    return !isFailed;
}

bool lockFreeQueuePopTest(size_t threadNum, int numberForOneThread, int repeatNum){
    auto lockFreeQueue = std::make_shared<lock_free_queue<int>>();
    std::vector<LockFreeQueueConsumerArgs*> consumersArgs;
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
        lockFreeQueue->push(i);
    }

    //lockFreeQueue->testQueue();

    auto start = std::chrono::steady_clock::now();
    for (int num = 0; num < repeatNum; ++num){
        for (size_t i = 0; i < threadNum; ++i){
            consumersArgs[num * threadNum + i] = new LockFreeQueueConsumerArgs{lockFreeQueue, checkingVec, numberForOneThread};
            int res = pthread_create(&thConsumers[num * threadNum + i], NULL, LockFreeQueueConsumer, (void*)consumersArgs[num * threadNum + i]);
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
    
    if (!lockFreeQueue->empty()){
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

bool lockFreeQueuePushPopTest(int number, int repeatNum){
    int maxThreadNum = get_nprocs_conf();
    std::cout << maxThreadNum << std::endl;
    int produserThreadNum;
    int consumerThreadNum;
    for (produserThreadNum = 1; produserThreadNum < maxThreadNum; ++produserThreadNum){
        for (consumerThreadNum = 1; consumerThreadNum + produserThreadNum <= maxThreadNum; ++consumerThreadNum){
            std::cout << produserThreadNum << ' ' << consumerThreadNum << std::endl;

            auto msmpQueue = std::make_shared<lock_free_queue<int>>();
            std::vector<LockFreeQueueProduserArgs*> produsersArgs(produserThreadNum * repeatNum);
            std::vector<pthread_t> thProdusers(produserThreadNum * repeatNum);
            std::vector<LockFreeQueueConsumerArgs*> consumersArgs(consumerThreadNum * repeatNum);
            std::vector<pthread_t> thConsumers(consumerThreadNum * repeatNum);
            #ifdef CPP11_ATOMIC
                std::vector<std::atomic<int>> checkingVec(number);
            #else
                std::vector<int> checkingVec(number);
            #endif

            for (int i = 0; i < number; ++i){
                checkingVec[i] = 0;
            }

            auto start = std::chrono::steady_clock::now();
            for (int num = 0; num < repeatNum; ++num){
                std::cout << "Num " << num << std::endl;
                for (int i = 0; i < produserThreadNum; ++i){
                    int startNum = (int)(i * (number / produserThreadNum));
                    int endNum;
                    if (i == produserThreadNum - 1)
                        endNum = number - 1;
                    else 
                        endNum = (int)((i + 1) * (number / produserThreadNum) - 1);
                    produsersArgs[num * produserThreadNum + i] = new LockFreeQueueProduserArgs{msmpQueue, startNum, endNum};
                    int res = pthread_create(&thProdusers[num * produserThreadNum + i], NULL, LockFreeQueueProduser, (void*)produsersArgs[num * produserThreadNum + i]);
                    if (res != 0)
                        std::cout << "ERROR: thread create failed. Number: " << num * produserThreadNum + i << std::endl;
                }
                for (int i = 0; i < consumerThreadNum; ++i){
                    int numberForOneThread;
                    if (i == produserThreadNum - 1)
                        numberForOneThread = number - number / consumerThreadNum * (consumerThreadNum - 1);
                    else
                        numberForOneThread = number / consumerThreadNum;
                    consumersArgs[num * consumerThreadNum + i] = new LockFreeQueueConsumerArgs{msmpQueue, checkingVec, numberForOneThread};
                    int res = pthread_create(&thConsumers[num * consumerThreadNum + i], NULL, LockFreeQueueConsumer, (void*)consumersArgs[num * consumerThreadNum + i]);
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
                std::cout << std::endl << std::endl;
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
            bool isFailed = false;
            for (int i = 0; i < number; ++i){
            #ifdef CPP11_ATOMIC
                if(checkingVec[i].load() != repeatNum){
                    //std::cout << "FAILED TEST: to many repeat " << i << " in queue " << checkingVec[i] << " numbers " << std::endl;
                    isFailed = true;
                }
            #else
                if(checkingVec[i] != repeatNum){
                    std::cout << "FAILED TEST: to many repeat " << i << " in queue" << std::endl;
                    isFailed = true;
                }
            #endif
            }
            if (isFailed){
                return !isFailed;
            }
        }
    }
    return true;
}