project(engine_core)

add_library(engine_core STATIC
    src/ecs/Registry.cpp
)

target_include_directories(engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)
