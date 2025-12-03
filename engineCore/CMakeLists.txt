cmake_minimum_required(VERSION 3.14)

project(engineCore)

add_library(engineCore STATIC
    src/ecs/World.cpp
)

target_include_directories(engineCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

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
