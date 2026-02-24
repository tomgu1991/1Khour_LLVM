llvm_map_components_to_libnames(llvm_libs Support)

add_llvm_library(
        IMPL STATIC
        SimpleCFGAction.cpp

        LINK_LIBS
        clangBasic
        clangLex
        clangAST
        clangSema
        clangFrontend
        clangTooling
        clangToolingRefactoring
        clangDriver
        ${llvm_libs}

        BUILDTREE_ONLY
)