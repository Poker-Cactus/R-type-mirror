project(network)

find_package(CapnProto REQUIRED)
find_package(asio REQUIRED)

capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS include/GameMessage.capnp)

add_library(network
    src/ANetworkManager.cpp
    src/capnpHandler.cpp
    src/asioServer.cpp
    src/asioClient.cpp
    ${CAPNP_SRCS}
    ${CAPNP_HDRS}
)

target_include_directories(network PUBLIC
    include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(network PUBLIC
    common
    asio::asio
    CapnProto::capnp
    CapnProto::kj
)
