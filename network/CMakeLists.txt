project(network)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601 -D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

find_package(CapnProto REQUIRED)
find_package(asio REQUIRED)

capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS include/GameMessage.capnp)

add_library(network
    src/ANetworkManager.cpp
    src/CapnpHandler.cpp
    src/AsioServer.cpp
    src/AsioClient.cpp
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
