project(server)

# capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS
    # include/GameMessage.capnp
# )

add_executable(server
    src/main.cpp
    src/server.cpp
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
