cmake_minimum_required(VERSION 3.15)
project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")

# Export compile commands for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies setup via Conan
find_package(SDL2 REQUIRED)
find_package(asio REQUIRED)
find_package(CapnProto REQUIRED)
find_package(doctest REQUIRED)

# Add subdirectories
add_subdirectory(engine_core)
add_subdirectory(common)
add_subdirectory(network)
add_subdirectory(server)
add_subdirectory(client)
# add_subdirectory(tests) # Tests are included in subdirectories

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
