cmake_minimum_required(VERSION 3.10)
project(certifiable_inference C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags for high-integrity C
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Library
add_library(certifiable_inference
    src/containers/deterministic_hash.c
)

# Test executable
add_executable(test_hash_basic
    tests/unit/test_hash_basic.c
)

target_link_libraries(test_hash_basic certifiable_inference)

# Enable testing
enable_testing()
add_test(NAME test_hash_basic COMMAND test_hash_basic)
