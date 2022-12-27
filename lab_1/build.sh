#!/bin/bash

pidFilePath="/var/run/lab1.pid"

[[ -f $pidFilePath ]] || sudo touch "$pidFilePath"

sudo chmod 666 "$pidFilePath"

cmake .
make
rm CMakeCache.txt Makefile cmake_install.cmake
FILE=lab_var6.cbp
[[ -f "$FILE" ]] && rm "$FILE"
rm -r CMakeFiles/