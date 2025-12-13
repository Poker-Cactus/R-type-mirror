project(server)

# capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS
    # include/GameMessage.capnp
# )

add_executable(server
    src/main.cpp
    src/Server.cpp
    src/Game.cpp

    src/systems/NetworkReceiveSystem.cpp
    src/systems/NetworkSendSystem.cpp
)

target_include_directories(server PRIVATE
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(server PRIVATE
    engineCore
    common
    network
    asio::asio
)
