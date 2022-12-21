cmake_minimum_required(VERSION 3.14)
project(lab3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")


add_executable(lab3 test.cpp MSMP_queue.hpp MSMP_queueTest.h MSMP_queueTest.cpp) 