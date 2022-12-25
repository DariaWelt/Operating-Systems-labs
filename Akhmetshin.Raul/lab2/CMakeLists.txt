cmake_minimum_required(VERSION 3.10)
project(Lab2)

set(CLIENT_SRC client)
set(HOST_SRC host)
set(CONNECTION connection)
set(MODEL model)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_COMPILER g++)


add_compile_options(-Wall -Werror)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

include_directories(${CONNECTION})
include_directories(${MODEL})

find_package (Threads)
find_library (LIBRT rt)

file(GLOB CONN_FILES ./connection/conn_*.cpp)

foreach(CONN_FILE ${CONN_FILES})
    get_filename_component(FILE_NAME ${CONN_FILE} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN ${FILE_NAME})
    
    add_executable(host_${CONN} ${HOST_SRC}/host.cpp ${HOST_SRC}/wolf.h ${HOST_SRC}/wolf.cpp
                        ${MODEL}/game.h ${MODEL}/semaphores.h ${HOST_SRC}/mainwindow.cpp
                        ${HOST_SRC}/mainwindow.ui
                        ${CONNECTION}/conn.h ${CONNECTION}/conn_${CONN}.cpp)

    target_link_libraries(host_${CONN} rt pthread Qt${QT_VERSION_MAJOR}::Widgets)

    add_executable(client_${CONN} ${CLIENT_SRC}/client.cpp ${CLIENT_SRC}/goat.h ${CLIENT_SRC}/goat.cpp
                        ${MODEL}/game.h ${MODEL}/semaphores.h
                        ${CONNECTION}/conn.h ${CONNECTION}/conn_${CONN}.cpp)

    target_link_libraries(client_${CONN} rt pthread )
endforeach()
