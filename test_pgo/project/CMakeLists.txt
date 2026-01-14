cmake_minimum_required(VERSION 3.10)
project(LargeProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/main.cpp
    src/module_01.cpp
    src/module_02.cpp
    src/module_03.cpp
    src/module_04.cpp
    src/module_05.cpp
    src/module_06.cpp
    src/module_07.cpp
    src/module_08.cpp
    src/module_09.cpp
    src/module_10.cpp
    src/module_11.cpp
    src/module_12.cpp
)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create executable
add_executable(main_program ${SOURCES})

# Add optimization flags for PGO
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
