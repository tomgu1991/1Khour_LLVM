#!/bin/bash

# Build script for LargeProject CMake project
# This script builds the CMake project and generates the executable

set -e  # Exit on any error

rm -rf build
mkdir build
CUR="$PWD/../../llvm-project/build/bin/"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=${CUR}/clang++ \
    -DCMAKE_C_COMPILER=${CUR}/clang
ninja -C build -j 1
