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

foreach(PATH_TO_CON ${CONNECTIONS})
    get_filename_component(FILE_NAME ${PATH_TO_CON} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})

    qt5_wrap_cpp(GUI_SRC gui/MainWindow.h)
    qt5_wrap_ui(GUI_UI gui/mainwindow.ui)

    add_executable(host_${CONN_NAME} ${GUI} ${GUI_UI} ${GUI_SRC} ${HOST}
            conn/conn.hpp conn/conn_${CONN_NAME}.cpp conn/conn_${CONN_NAME}.hpp)
    add_executable(client_${CONN_NAME} ${GUI} ${GUI_UI} ${GUI_SRC} ${CLIENT}
            conn/conn.hpp conn/conn_${CONN_NAME}.cpp conn/conn_${CONN_NAME}.h host/host.hpp)
    target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt Qt5::Widgets)
    target_link_libraries(client_${CONN_NAME} PUBLIC pthread rt Qt5::Widgets)
endforeach()
