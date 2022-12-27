cmake_minimum_required(VERSION 3.16)
project(daemon)

set(CMAKE_CXX_STANDARD 17)
file(GLOB SRC src/*.cpp src/*.h)
add_compile_options(-Wall -Wextra -pedantic -Werror)

add_executable(${PROJECT_NAME} daemon_main.cpp ${SRC})