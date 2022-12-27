cmake_minimum_required(VERSION 3.16)
project(lab1)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror)

add_executable(${PROJECT_NAME} main.cpp ConfigParser.cpp ConfigParser.h Daemon.cpp Daemon.h)
