project(flappybird_client)

add_executable(flappybird_client
    src/main.cpp
    src/Game.cpp
    src/Menu.cpp
    src/Lobby.cpp
)

target_include_directories(flappybird_client PRIVATE
    ${CMAKE_SOURCE_DIR}/flappyBird/client/src
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/client
)

target_link_libraries(flappybird_client PRIVATE
    engineCore
    common
    network
    SDL2::SDL2main
    SDL2::SDL2
)

# Link macOS frameworks if needed
if(APPLE)
    target_link_libraries(flappybird_client PRIVATE
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework IOKit"
    )
endif()

# Ensure SDL2main handles the main/WinMain conversion on Windows
if(WIN32)
    set_target_properties(flappybird_client PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Set output directory
set_target_properties(flappybird_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/flappyBird/client
)
