#!/usr/bin/env bash

set -x

# export LDFLAGS="-fuse-ld=lld"
# export CC=clang
# export CXX=clang++

CUR_DIR=`pwd`

#git clone git@github.com:llvm/llvm-project.git
#git checkout llvmorg-20.1.0

ROOT=$CUR_DIR/llvm-project
LLVM_ROOT=$ROOT/llvm
BUILD_DIR=$ROOT/build
INSTALL_DIR=$ROOT/install/release

cd $ROOT
if [ ! -d build ]; then
    mkdir $BUILD_DIR
fi

cmake -S $LLVM_ROOT -B $BUILD_DIR -G "Ninja" -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra;" \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_INSTALL_UTILS=ON \
  -DLLVM_ENABLE_RUNTIMES=compiler-rt
cd $BUILD_DIR
ninja -j12
ninja install

