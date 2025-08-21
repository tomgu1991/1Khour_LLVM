cmake_minimum_required(VERSION 3.20)
project(Kal_tutorial)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# LLVM Config
# locate LLVM and Clang packages
find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
message(STATUS "[+] Using LLVM version ${LLVM_VERSION}")
# append llvm dir, so that CMake can find necessary files to configure LLVM
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
# macros and functions for custom extension, locate at llvm/cmake/modules/XXX.cmake
include(AddLLVM) # llvm_add_library
include(HandleLLVMOptions) # LLVM_ENABLE_ASSERTIONS
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
message(STATUS "[+] LLVM includes ${LLVM_INCLUDE_DIRS}")
include_directories(SYSTEM ${CLANG_INCLUDE_DIRS})
message(STATUS "[+] Clang includes ${CLANG_INCLUDE_DIRS}")
add_definitions(${LLVM_DEFINITIONS})

include_directories(include)

add_executable(Kal_lang main.cpp)
llvm_map_components_to_libnames(llvm_libs Core Support Analysis Passes)
target_link_libraries(Kal_lang ${llvm_libs})
