cd ..
cd llvm-project
cmake -S llvm -B build_clang_pgo_version -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_PROFDATA_FILE="../test_pgo/project/test.profdata"
ninja -C build_clang_pgo_version clang llvm-profdata