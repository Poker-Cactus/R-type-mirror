project(client)

add_executable(client
    src/main.cpp
    src/Client.cpp
    src/systems/NetworkSendSystem.cpp
    src/systems/NetworkReceiveSystem.cpp
    src/Game.cpp
    src/Menu.cpp
    src/PlayingState.cpp
    src/ParallaxBackground.cpp
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

add_subdirectory(modules)
