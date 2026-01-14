project(flappybird_server)

add_executable(flappybird_server
    src/main.cpp
    src/Game.cpp
    
    src/systems/NetworkReceiveSystem.cpp
    src/systems/NetworkSendSystem.cpp
)

target_include_directories(flappybird_server PRIVATE
    include
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(flappybird_server PRIVATE
    engineCore
    common
    network
    asio::asio
    nlohmann_json::nlohmann_json
)

# Set output directory
set_target_properties(flappybird_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/flappyBird/server
)
