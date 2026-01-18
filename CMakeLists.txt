cmake_minimum_required(VERSION 3.10)
project(certifiable_inference C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags for high-integrity C
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")

# Additional flags for static analysis
include(CheckCCompilerFlag)

check_c_compiler_flag("-fanalyzer" HAS_FANALYZER)
if (HAS_FANALYZER)
  add_compile_options(-fanalyzer)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/core)
include_directories(${PROJECT_SOURCE_DIR}/include/containers)

# Core library sources
add_library(certifiable_inference
    src/containers/deterministic_hash.c
    src/core/fixed_point.c
    src/core/matrix.c
    src/core/activations.c
    src/core/convolution.c
    src/core/pooling.c
)

# Example programs
add_executable(xor_gate
    examples/xor_gate.c
)
target_link_libraries(xor_gate certifiable_inference m)

add_executable(edge_detection
    examples/edge_detection.c
)
target_link_libraries(edge_detection certifiable_inference m)

# Benchmarks
add_executable(timing_benchmark
    tests/benchmarks/test_timing_consistency.c
)
target_link_libraries(timing_benchmark certifiable_inference m)

# Enable testing
enable_testing()

function(ci_add_unit_test name)
  add_executable(${name} ${ARGN})

  # -UNDEBUG undefines NDEBUG even if it came from -DNDEBUG in Release.
  target_compile_options(${name} PRIVATE -UNDEBUG)

  # Common libs for all unit tests
  target_link_libraries(${name} PRIVATE certifiable_inference m)

  # Register with CTest
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# Unit test executables
ci_add_unit_test(test_hash_basic              tests/unit/test_hash_basic.c)
ci_add_unit_test(test_hash_consistency        tests/unit/test_hash_consistency.c)
ci_add_unit_test(test_fixed_point             tests/unit/test_fixed_point.c)
ci_add_unit_test(test_matrix_reproducibility  tests/unit/test_matrix_reproducibility.c)
ci_add_unit_test(test_activations             tests/unit/test_activations.c)
ci_add_unit_test(test_convolution             tests/unit/test_convolution.c)
ci_add_unit_test(test_pooling                 tests/unit/test_pooling.c)

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
    DEPENDS test_hash_basic
            test_hash_consistency
            test_fixed_point
            test_matrix_reproducibility
            test_activations
            test_convolution
            test_pooling
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

# Custom target for benchmarks
add_custom_target(
    benchmarks
    COMMAND ./timing_benchmark
    DEPENDS timing_benchmark
    COMMENT "Running performance benchmarks"
)

# Print configuration summary
message(STATUS "")
message(STATUS "═══════════════════════════════════════════════")
message(STATUS "  Certifiable Inference Engine Configuration")
message(STATUS "═══════════════════════════════════════════════")
message(STATUS "  C Compiler:    ${CMAKE_C_COMPILER}")
message(STATUS "  C Standard:    C99")
message(STATUS "  Build Type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Flags:       ${CMAKE_C_FLAGS}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  ✓ Fixed-point arithmetic (Q16.16)")
message(STATUS "  ✓ Matrix operations")
message(STATUS "  ✓ Convolution (2D)")
message(STATUS "  ✓ Activation functions (ReLU)")
message(STATUS "  ✓ Max Pooling (2×2 stride-2)")
message(STATUS "  ✓ Deterministic hash table")
message(STATUS "")
message(STATUS "Tests:")
message(STATUS "  ✓ Unit tests (7 test suites)")
message(STATUS "  ✓ Timing benchmarks")
message(STATUS "  ✓ Example programs (xor_gate, edge_detection)")
message(STATUS "")
if(CPPCHECK)
    message(STATUS "Static Analysis:")
    message(STATUS "  ✓ cppcheck: Available")
else()
    message(STATUS "Static Analysis:")
    message(STATUS "  ✗ cppcheck: Not found (install for static analysis)")
endif()
message(STATUS "")
message(STATUS "Build Targets:")
message(STATUS "  make              - Build all components")
message(STATUS "  make test-all     - Run all unit tests")
message(STATUS "  make benchmarks   - Run timing benchmarks")
message(STATUS "  make verify-all   - Run tests + static analysis")
if(CPPCHECK)
    message(STATUS "  make cppcheck     - Run cppcheck only")
endif()
if(CPPCHECK_MISRA)
    message(STATUS "  make misra-check  - Run MISRA-C compliance check")
endif()
message(STATUS "═══════════════════════════════════════════════")
message(STATUS "")

# Debug programs
add_executable(debug_conv examples/debug_conv.c)
target_link_libraries(debug_conv certifiable_inference m)

add_executable(debug_pool examples/debug_pool.c)
target_link_libraries(debug_pool certifiable_inference m)
