#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
#include "MSMP_queueTest.h"
#include "lockFreeQueueTest.h"

int main(int argc, char** argv){
    if (argc != 2) {
        std::cerr << "Incorrect arguments. Provide only one argument - configuration file." << std::endl;
        return EXIT_FAILURE;
    }


    std::ifstream configFile(argv[1]);
    if (!configFile.is_open()) {
        std::cout << "Invalid config" << std::endl;
        return 0;
    }

    std::regex commentFmt(R"(^\s*#.*\s*)"),
        pushTestFmt(R"(^\s*PushTest:\s*(\d+)\s*(\d+)\s*(\d+)\s*$)"),
        popTestFmt(R"(^\s*PopTest:\s*(\d+)\s*(\d+)\s*(\d+)\s*$)"),
        pushPopTestFmt(R"(^\s*PushPopTest:\s*(\d+)\s*(\d+)\s*$)"),
        emptyFmt(R"(^\s*$)");

    bool testRes;

    std::string line;
    while (std::getline(configFile, line)) {
        if (std::regex_match(line, commentFmt)){
            std::cout << line << std::endl;
        }
        else if (std::regex_match(line, pushTestFmt)){
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), pushTestFmt), std::sregex_iterator());

            int threadNum = atoi(matches[0][1].str().c_str());
            int numberForOneThread =  atoi(matches[0][2].str().c_str());
            int repeatNum = atoi(matches[0][3].str().c_str());
            
            std::cout << "Queue with lock test:" << std::endl;
            testRes = MSMPQueuePushTest(std::size_t(threadNum), numberForOneThread, repeatNum);
            if (testRes == true)
                std::cout << "Sucsess test" << std::endl;
            else
                std::cout << "Failed test" << std::endl;
            std::cout << "Lock-free queue test:" << std::endl;
            testRes = lockFreeQueuePushTest(std::size_t(threadNum), numberForOneThread, repeatNum);
            if (testRes == true)
                std::cout << "Sucsess test" << std::endl;
            else
                std::cout << "Failed test" << std::endl;
        }
        else if (std::regex_match(line, popTestFmt)){
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), popTestFmt), std::sregex_iterator());

            int threadNum = atoi(matches[0][1].str().c_str());
            int numberForOneThread =  atoi(matches[0][2].str().c_str());
            int repeatNum = atoi(matches[0][3].str().c_str());
            
            std::cout << "Queue with lock test:" << std::endl;
            testRes = MSMPQueuePopTest(std::size_t(threadNum), numberForOneThread, repeatNum);
            if (testRes == true)
                std::cout << "Sucsess test" << std::endl;
            else
                std::cout << "Failed test" << std::endl;
            std::cout << "Lock-free queue test:" << std::endl;
            testRes = lockFreeQueuePopTest(std::size_t(threadNum), numberForOneThread, repeatNum);
            if (testRes == true)
                std::cout << "Sucsess test" << std::endl;
            else
                std::cout << "Failed test" << std::endl;
        }
        else if ((std::regex_match(line, pushPopTestFmt))){
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), pushPopTestFmt), std::sregex_iterator());
            int number =  atoi(matches[0][1].str().c_str());
            int repeatNum = atoi(matches[0][2].str().c_str());
            
            std::cout << "Queue with lock test:" << std::endl;
            testRes = MSMPQueuePushPopTest(number, repeatNum);
            if (testRes == true)
                std::cout << "Sucsess test" << std::endl;
            else
                std::cout << "Failed test" << std::endl;
            std::cout << "Lock-free queue test:" << std::endl;
            testRes = lockFreeQueuePushPopTest(number, repeatNum);
            if (testRes == true)
                std::cout << "Sucsess test" << std::endl;
            else
                std::cout << "Failed test" << std::endl;
        }
        else if ((std::regex_match(line, emptyFmt))){
            //ignore empty line
        }
        else {
            std::cout << "Failed to parse string: \"" << line << "\"\nIt will be ignored" << std::endl;
        }
    }
}