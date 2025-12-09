project(client)

add_executable(client
    src/main.cpp
    src/client.cpp
)

target_include_directories(client PRIVATE
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(client PRIVATE
    engineCore
    common
    network
    SDL2::SDL2
)
