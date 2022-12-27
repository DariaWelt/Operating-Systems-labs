#!/bin/sh

mkdir "build"

cmake -S ./ -B build

cd build

cmake --build .

mv host* ../
mv client* ../

cd ..

rm -r build

