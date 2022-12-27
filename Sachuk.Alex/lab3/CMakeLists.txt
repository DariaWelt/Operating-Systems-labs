cmake_minimum_required (VERSION 3.8)

project ("lab3_3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror)

find_library(LIBRT rt)
find_package(Threads)

add_executable (lab3 main.cpp)
target_link_libraries(lab3 PUBLIC pthread rt)
