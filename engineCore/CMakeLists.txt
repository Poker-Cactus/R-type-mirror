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

# Add coverage flags if enabled
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(engineCore PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(engineCore PRIVATE --coverage)
        message(STATUS "Code coverage enabled for engineCore")
    endif()
endif()

# Enable testing at the project level
enable_testing()

# Add examples subdirectory if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

# Add tests subdirectory if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif()
