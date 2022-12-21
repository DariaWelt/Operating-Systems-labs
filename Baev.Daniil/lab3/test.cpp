#include <iostream>

#include "MSMP_queueTest.h"

int main(void){
    bool testRes;

    std::cout << "MSMP Queue:" << std::endl;

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
        std::cout << "Failed test" << std::endl;
    return 0;
}