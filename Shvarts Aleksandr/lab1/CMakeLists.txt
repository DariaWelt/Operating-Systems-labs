cmake_minimum_required(VERSION 3.22)
project(watchdog)

set(CMAKE_CXX_STANDARD 17)


add_compile_options(-Wall -Werror)
add_executable(${PROJECT_NAME} main.cpp Daemon.cpp Notify.cpp)
