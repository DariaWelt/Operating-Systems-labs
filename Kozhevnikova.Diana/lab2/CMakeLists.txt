cmake_minimum_required(VERSION 3.16)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package (Threads)
find_library (LIBRT rt)

file(GLOB CONNECTIONS connections/conn_*.h)
file(GLOB GUI gui/*.ui gui/*.cpp gui/*.h )

file(GLOB HOST queueMsg.h host/*.cpp host/*.h)
file(GLOB CLIENT queueMsg.h client/*.cpp client/*.h)

foreach(PATH_TO_CON ${CONNECTIONS})
    get_filename_component(FILE_NAME ${PATH_TO_CON} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})

    qt5_wrap_cpp(GUI_SRC gui/gui.h)
    qt5_wrap_ui(GUI_UI gui/windowChat.ui)

    add_executable(host_${CONN_NAME} 
            ${GUI} ${GUI_UI} ${GUI_SRC} 
            ${HOST} ${CLIENT}
            connections/conn.h connections/conn_${CONN_NAME}.cpp connections/conn_${CONN_NAME}.h)
            
    target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt Qt5::Widgets)
endforeach()