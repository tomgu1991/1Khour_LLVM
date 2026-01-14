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