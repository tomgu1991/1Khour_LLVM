# 编译加速

通过PGO clang编译器从而提高编译速度。

1. 用Mac自带的编译器编译
```
/usr/bin/time bash build_system.sh
3.14 real         2.56 user         0.59 sys
```

2. 用llvm-20.1的clang编译
```
> /usr/bin/time bash build_clang_20.sh
4.28 real         3.66 user         0.62 sys
```

3. 编译插桩版本的clang-20
```
# build_instr_clang.sh
> /usr/bin/time bash build_clang_instr.sh
 6.22 real         5.09 user         0.98 sys
profiels目录有内容
../../llvm-project/build_clang_instr/bin/llvm-profdata merge -output=test.profdata profiles/*.profraw
```

4. 获得profdata并优化
```
../../llvm-project/build_clang_instr/bin/llvm-profdata merge -output=test.profdata profiles/*.profraw
bash build_pgo_clang.sh
```

5. 用pgo后的版本
```
# build_pgo_clang.sh
> /usr/bin/time bash build_clang_pgo.sh
3.31 real         2.68 user         0.59 sys
```


## 编译clang测试
1. linux: 原始clang17
```
rm -rf ori_build
mkdir ori_build && cd ori_build
llvm_use_linker="-DLLVM_USE_LINKER=lld"

cmake -G "Ninja" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    $llvm_use_linker \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="AArch64;ARM;X86" \
    -DLLVM_INSTALL_UTILS=ON \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    ../llvm
set +x
JOBCNT=$(nproc) && (( JOBCNT > 4 )) && (( JOBCNT -= 4 )) || (( JOBCNT = 1 ))
ninja -j${JOBCNT}
[3891/3891] Generating ../../bin/llvm-readelf
21253.39user 711.39system 3:48.57elapsed 9609%CPU (0avgtext+0avgdata 1053908maxresident)k
6648inputs+6287240outputs (1620major+172448545minor)pagefaults 0swaps
```

2. 编译插桩版本clang: 5m 1s
```
rm -rf clang_instr_gen_build
mkdir clang_instr_gen_build && cd clang_instr_gen_build
llvm_use_linker="-DLLVM_USE_LINKER=lld"

cmake -G "Ninja" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    $llvm_use_linker \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="AArch64;ARM;X86" \
    -DLLVM_INSTALL_UTILS=ON \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-fprofile-instr-generate" \
    -DCMAKE_CXX_FLAGS="-fprofile-instr-generate" \
    ../llvm
set +x
JOBCNT=$(nproc) && (( JOBCNT > 4 )) && (( JOBCNT -= 4 )) || (( JOBCNT = 1 ))
ninja -j${JOBCNT}
```

3. 生成profdata: 17m 4s
```
rm -rf clang_gen_profdata
mkdir clang_gen_profdata && cd clang_gen_profdata
llvm_use_linker="-DLLVM_USE_LINKER=lld"

export LLVM_PROFILE_FILE="$PWD/profiles/clang-%m-%p.profraw"
rm -rf profiles
mkdir -p profiles

cmake -G "Ninja" \
    -DCMAKE_C_COMPILER=/data/home/zuxinggu/code/llvm-17/clang_instr_gen_build/bin/clang \
    -DCMAKE_CXX_COMPILER=/data/home/zuxinggu/code/llvm-17/clang_instr_gen_build/bin/clang++ \
    $llvm_use_linker \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="AArch64;ARM;X86" \
    -DLLVM_INSTALL_UTILS=ON \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    ../llvm
set +x
JOBCNT=$(nproc) && (( JOBCNT > 4 )) && (( JOBCNT -= 4 )) || (( JOBCNT = 1 ))
ninja -j${JOBCNT}

cd clang_gen_profdata
# 3800+ 很慢，选500个
../install/release/bin/llvm-profdata merge -output=clang.profdata \
  $(ls -1 profiles/*.profraw | head -n 500)
```

4. pgo - clang: 3m 58s
```
rm -rf build_pgo_version
mkdir build_pgo_version && cd build_pgo_version
llvm_use_linker="-DLLVM_USE_LINKER=lld"

cmake -G "Ninja" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    $llvm_use_linker \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="AArch64;ARM;X86" \
    -DLLVM_INSTALL_UTILS=ON \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_PROFDATA_FILE="/data/home/zuxinggu/code/llvm-17/clang_gen_profdata/clang.profdata" \
    ../llvm
set +x
JOBCNT=$(nproc) && (( JOBCNT > 4 )) && (( JOBCNT -= 4 )) || (( JOBCNT = 1 ))
ninja -j${JOBCNT}
warning: no profile data available for file "Scalar.cpp" [-Wprofile-instr-unprofiled]
1 warning generated.
[1186/3891] Building CXX object lib/DWARFLinker/CMakeFiles/LLVMDWARFLinker.dir/DWARFLinker.cpp.o
warning: no profile data available for file "DWARFLinker.cpp" [-Wprofile-instr-unprofi
```

5. pgo-version编译
```
rm -rf ori_build
mkdir ori_build && cd ori_build
llvm_use_linker="-DLLVM_USE_LINKER=lld"

cmake -G "Ninja" \
    -DCMAKE_C_COMPILER=/data/home/zuxinggu/code/llvm-17/build_pgo_version/bin/clang \
    -DCMAKE_CXX_COMPILER=/data/home/zuxinggu/code/llvm-17/build_pgo_version/bin/clang++ \
    $llvm_use_linker \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="AArch64;ARM;X86" \
    -DLLVM_INSTALL_UTILS=ON \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    ../llvm
set +x
JOBCNT=$(nproc) && (( JOBCNT > 4 )) && (( JOBCNT -= 4 )) || (( JOBCNT = 1 ))
ninja -j${JOBCNT}

14172.45user 635.84system 2:48.09elapsed 8809%CPU (0avgtext+0avgdata 1054292maxresident)k
17880inputs+6475720outputs (1742major+161799238minor)pagefaults 0swaps
```