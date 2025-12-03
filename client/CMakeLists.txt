project(client)

capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS include/GameMessage.capnp)

add_executable(client
    src/main.cpp
    src/client.cpp
    ${CAPNP_SRCS}
)

target_include_directories(client PRIVATE
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(client PRIVATE 
    engine_core 
    common 
    SDL2::SDL2
    CapnProto::capnp
    CapnProto::kj
)
