#!/usr/bin/env bash

set -e
set -x

SCRIPT_PATH=$(cd `dirname $0`; pwd)
# llvm/build_release_dir
LLVM_PATH=$SCRIPT_PATH/../llvm-project/install/release

mkdir -p build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=debug -DCMAKE_PREFIX_PATH=$LLVM_PATH \
  -DCMAKE_INSTALL_PREFIX=$SCRIPT_PATH/install/debug $SCRIPT_PATH

ninja -j10
ninja install

cd $SCRIPT_PATH
./install/debug/bin/example -count-func-name test/case.cpp -- -g