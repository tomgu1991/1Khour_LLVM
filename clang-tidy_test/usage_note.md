https://releases.llvm.org/20.1.0/tools/clang/tools/extra/docs/clang-tidy/index.html

```
../llvm-project/install/bin/clang-tidy --version
# 打印所有的规则
../llvm-project/install/bin/clang-tidy -list-checks -checks='*'
../llvm-project/install/bin/clang-tidy test.cpp -checks='*,-llvmlibc*' -- -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

```