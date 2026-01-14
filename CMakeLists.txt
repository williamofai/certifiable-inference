cmake_minimum_required(VERSION 3.10)
project(certifiable_inference C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags for high-integrity C
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")

# Additional flags for static analysis
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fanalyzer")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Library
add_library(certifiable_inference
    src/containers/deterministic_hash.c
    src/core/fixed_point.c
)

# Test executables
add_executable(test_hash_basic
    tests/unit/test_hash_basic.c
)
target_link_libraries(test_hash_basic certifiable_inference)

add_executable(test_hash_consistency
    tests/unit/test_hash_consistency.c
)
target_link_libraries(test_hash_consistency certifiable_inference)

add_executable(test_fixed_point
    tests/unit/test_fixed_point.c
)
target_link_libraries(test_fixed_point certifiable_inference m)

# Enable testing
enable_testing()
add_test(NAME test_hash_basic COMMAND test_hash_basic)
add_test(NAME test_hash_consistency COMMAND test_hash_consistency)
add_test(NAME test_fixed_point COMMAND test_fixed_point)

# Static Analysis Targets
find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK}
            --enable=all
            --error-exitcode=1
            --suppress=missingIncludeSystem
            --std=c99
            -I ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/
        COMMENT "Running cppcheck static analysis"
    )
endif()

# MISRA-C checking (if cppcheck-misra is available)
find_program(CPPCHECK_MISRA cppcheck)
if(CPPCHECK_MISRA)
    add_custom_target(
        misra-check
        COMMAND ${CPPCHECK_MISRA}
            --addon=misra
            --error-exitcode=1
            --suppress=missingIncludeSystem
            -I ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/
        COMMENT "Running MISRA-C compliance check"
    )
endif()

# Coverage target (requires gcov)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Custom target to run all tests
add_custom_target(
    test-all
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_hash_basic test_hash_consistency test_fixed_point
    COMMENT "Running all tests"
)

# Custom target to run static analysis and tests
add_custom_target(
    verify-all
    DEPENDS test-all
    COMMENT "Running complete verification suite"
)

if(CPPCHECK)
    add_dependencies(verify-all cppcheck)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Certifiable Inference Configuration:")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Standard: C99")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
if(CPPCHECK)
    message(STATUS "  cppcheck: Found")
else()
    message(STATUS "  cppcheck: Not found (install for static analysis)")
endif()
message(STATUS "")
