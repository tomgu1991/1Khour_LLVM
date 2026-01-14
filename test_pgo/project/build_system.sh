#!/bin/bash

# Build script for LargeProject CMake project
# This script builds the CMake project and generates the executable

set -e  # Exit on any error

rm -rf build
mkdir build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build -j 1
