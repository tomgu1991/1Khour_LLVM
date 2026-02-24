cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

project(LibtoolingExample)


find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)
include(HandleLLVMOptions)

set(llvm_include_dirs
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

include_directories(include)
include_directories(SYSTEM ${llvm_include_dirs})
add_subdirectory(lib)


add_llvm_executable(example
        main.cpp
)

target_link_libraries(example PRIVATE
        IMPL
        clangBasic
        clangLex
        clangAST
        clangSema
        clangFrontend
        clangTooling
        clangToolingRefactoring
        clangDriver

        # Revision [1] in clang moved PCHContainerOperations from Frontend
        # to Serialization, but this broke builds that set
        # -DBUILD_SHARED_LIBS=on.  Revision [2] is a followup that works
        # around the issue by adding an explicit dependency on Serialization
        # wherever there was a dependency on Frontend.  Since we depend on
        # Frontend, we need an explicit dependency on Serialization too.
        # [1] https://llvm.org/viewvc/llvm-project?view=revision&revision=348907
        # [2] https://llvm.org/viewvc/llvm-project?view=revision&revision=348915
        clangSerialization

        clangToolingInclusionsStdlib
)

install(TARGETS example RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})