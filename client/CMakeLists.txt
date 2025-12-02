project(client)

add_executable(client
    src/main.cpp
    src/client.cpp
)

target_include_directories(client PRIVATE include)

target_link_libraries(client PRIVATE 
    engine_core 
    common 
    SDL2::SDL2
)
