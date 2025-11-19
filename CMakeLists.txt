cmake_minimum_required(VERSION 3.15)
project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies setup via Conan
find_package(SDL2 REQUIRED)
find_package(asio REQUIRED)

# Add subdirectories
add_subdirectory(engine_core)
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
