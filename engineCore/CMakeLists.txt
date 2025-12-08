cmake_minimum_required(VERSION 3.14)

project(engineCore)

# If building standalone and doctest is in parent project's build dir
if(NOT CMAKE_PREFIX_PATH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../build")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build")
    message(STATUS "Added parent build directory to CMAKE_PREFIX_PATH for standalone build")
endif()

# Option for code coverage
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Header-only library
add_library(engineCore INTERFACE)

target_include_directories(engineCore INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

# Note: Strict compilation flags are applied at the test level for header-only library

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
