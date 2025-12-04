cmake_minimum_required(VERSION 3.14)

project(engineCore)

# If building standalone and doctest is in parent project's build dir
if(NOT CMAKE_PREFIX_PATH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../build")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build")
    message(STATUS "Added parent build directory to CMAKE_PREFIX_PATH for standalone build")
endif()

# Option for code coverage
option(ENABLE_COVERAGE "Enable code coverage" OFF)

add_library(engineCore STATIC
    src/ecs/World.cpp
)

target_include_directories(engineCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

# Strict compilation flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engineCore PRIVATE
        -Wall              # Enable all common warnings
        -Wextra            # Enable extra warnings
        -Wpedantic         # Strict ISO C++ compliance
        # -Werror          # Treat warnings as errors (disabled to allow compilation)
        -Wshadow           # Warn about variable shadowing
        -Wcast-align       # Warn about pointer cast alignment issues
        -Wunused           # Warn about unused variables/functions
        # -Wconversion     # Warn about implicit conversions (can be noisy)
        # -Wsign-conversion # Warn about sign conversions (can be noisy)
        -Wnull-dereference # Warn about null pointer dereferences
        -Wdouble-promotion # Warn about float->double promotions
        -Wformat=2         # Extra format string checks
    )

    # Additional flags for GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(engineCore PRIVATE
            -Wduplicated-cond     # Warn about duplicated conditions
            -Wduplicated-branches # Warn about duplicated branches
            -Wlogical-op          # Warn about logical operations
        )
    endif()

    # Additional flags for Clang
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(engineCore PRIVATE
            -Wmost                # Most warnings
        )
    endif()
elseif(MSVC)
    target_compile_options(engineCore PRIVATE
        /W4      # Warning level 4
        # /WX    # Treat warnings as errors (disabled to allow compilation)
        /permissive- # Strict standard conformance
    )
endif()

# Add coverage flags if enabled
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(engineCore PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(engineCore PRIVATE --coverage)
        message(STATUS "Code coverage enabled for engineCore")
    endif()
endif()

# Options to control building of tests and examples
option(BUILD_TESTS "Build the tests" OFF)
option(BUILD_EXAMPLES "Build the examples" OFF)

# Enable testing at the project level only if building tests
if(BUILD_TESTS)
    enable_testing()
endif()

# Add examples subdirectory if it exists and BUILD_EXAMPLES is ON
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

# Add tests subdirectory if it exists and BUILD_TESTS is ON
if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif()
