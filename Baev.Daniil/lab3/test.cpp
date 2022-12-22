#include <iostream>

#include "MSMP_queueTest.h"
#include "lockFreeQueueTest.h"

int main(void){
    bool testRes;

    /*std::cout << "Queue with lock:" << std::endl;

    std::cout << "Easy push test" << std::endl;
    testRes = MSMPQueuePushTest(std::size_t(10), 10000, 1);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;

    std::cout << "Easy pop test" << std::endl;
    testRes = MSMPQueuePopTest(std::size_t(10), 10000, 1);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;



    std::cout << "Push time test" << std::endl;
    testRes = MSMPQueuePushTest(std::size_t(10), 10000, 100);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;

    std::cout << "Pop time test" << std::endl;
    testRes = MSMPQueuePopTest(std::size_t(10), 10000, 100);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;

    std::cout << "Push pop time test" << std::endl;
    testRes = MSMPQueuePushPopTest(10000, 100);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;*/

    

    std::cout << "Queue lock-free:" << std::endl;

    std::cout << "Easy pop test" << std::endl;
    testRes = lockFreeQueuePopTest(std::size_t(5), 1000, 1);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;

    std::cout << "Easy push test" << std::endl;
    testRes = lockFreeQueuePushTest(std::size_t(5), 1000, 1);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;


    std::cout << "Push time test" << std::endl;
    testRes = lockFreeQueuePushTest(std::size_t(10), 10000, 100);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;

    std::cout << "Pop time test" << std::endl;
    testRes = lockFreeQueuePopTest(std::size_t(10), 10000, 100);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;

    std::cout << "Push pop time test" << std::endl;
    testRes = lockFreeQueuePushPopTest(10000, 100);
    if (testRes == true)
        std::cout << "Sucsess test" << std::endl;
    else
        std::cout << "Failed test" << std::endl;
    return 0;
}