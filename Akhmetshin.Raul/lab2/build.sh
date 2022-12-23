#!/bin/sh

mkdir build

cmake -B ./build -S ./
cd build
make

for conn_type in "shm" "mq" "fifo"
do
    cp -a ./"host_${conn_type}" ./../
    cp -a ./"client_${conn_type}" ./../
done

cd ..
rm -r build
