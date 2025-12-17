project(common)

add_library(common STATIC
    src/components/Components.cpp
    src/protocol/Protocol.cpp
)

target_include_directories(common PUBLIC include)

target_link_libraries(common PUBLIC asio::asio)
