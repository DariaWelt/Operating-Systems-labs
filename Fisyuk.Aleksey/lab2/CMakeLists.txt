cmake_minimum_required(VERSION 3.15)
project(lab2)
# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package (Threads)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_library (LIBRT rt)

file(GLOB CONNECTIONS src/conn/conn_*.cpp)
file(GLOB HOST src/host/*.cpp src/host/*.hpp)
file(GLOB CLIENT src/client/*.cpp src/client/*.hpp)
file(GLOB GUI src/gui/*.ui src/gui/*.cpp src/gui/*.h)

foreach(PATH_TO_CON ${CONNECTIONS})
        get_filename_component(FILE_NAME ${PATH_TO_CON} NAME_WLE )
        string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})
        
        qt5_wrap_cpp(GUI_SRC src/gui/ChatWindow.h)
        qt5_wrap_ui(GUI_UI src/gui/ChatWindow.ui)

        add_executable(host_${CONN_NAME} ${GUI} ${GUI_UI} ${GUI_SRC}
                ${HOST}   
                src/conn/IConn.hpp
                src/conn/conn_${CONN_NAME}.cpp )

        add_executable(client_${CONN_NAME} ${GUI} ${GUI_UI} ${GUI_SRC}
                ${CLIENT}
                src/conn/IConn.hpp 
                src/conn/conn_${CONN_NAME}.cpp )
        
        target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt Qt5::Widgets)
        target_link_libraries(client_${CONN_NAME} PUBLIC pthread rt Qt5::Widgets)
endforeach()