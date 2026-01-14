cd ..
cd llvm-project
cmake -S llvm -B build_clang_instr -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_BUILD_INSTRUMENTED=IR
ninja -C build_clang_instr clang llvm-profdata