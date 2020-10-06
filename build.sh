#!/usr/bin/env bash

mkdir build
cd build
# rm CMakeCache.txt  CMakeFiles/  cmake_install.cmake  compile_commands.json Makefile -r
CC=clang CXX=clang++ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
make -j4
