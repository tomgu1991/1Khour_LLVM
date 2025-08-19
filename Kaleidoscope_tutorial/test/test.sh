#!/usr/bin/env bash

LLVM_INSTALL=../../../llvm-17/install/release/bin/

$LLVM_INSTALL/llc --version
$LLVM_INSTALL/llc kal.ll -filetype=obj -o kal.o
clang++ main.cpp kal.o -o main
./main