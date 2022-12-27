project(wolf-and-goats)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(host client conn utils gui)
FILE(GLOB HOST host/*.cpp)
FILE(GLOB CONN conn/conn_*.cpp)
FILE(GLOB GUI gui/*.ui gui/*.cpp gui/*.h )


foreach(CONN_PATH ${CONN})
    get_filename_component(CONN_FILE ${CONN_PATH} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_TYPE ${CONN_FILE})
    add_executable(${PROJECT_NAME}-${CONN_TYPE} ${HOST} ${CONN_PATH})
    target_link_libraries(${PROJECT_NAME}-${CONN_TYPE} Threads::Threads rt)
endforeach()
