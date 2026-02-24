#!/usr/bin/env bash

set -x

rm -rf build
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=../llvm-project/install/release ..
ninja -v -j 10

echo "finished"

./Kal_lang
clang++ ../test/main.cpp output.o -o test_main
./test_main