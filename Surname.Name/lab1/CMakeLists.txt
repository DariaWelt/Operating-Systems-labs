cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(Deleter)

add_executable(Deleter main.cpp Deleter.cpp Deleter.h
        ConfigItem.cpp ConfigItem.h
        ConfigReader.cpp ConfigReader.h)