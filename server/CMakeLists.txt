project(server)

add_executable(server
    src/main.cpp
)

target_include_directories(server PRIVATE include)

target_link_libraries(server PRIVATE 
    engine_core 
    common 
    asio::asio
)
