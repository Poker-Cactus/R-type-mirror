project(client)

add_executable(client
    src/main.cpp
    src/Game.cpp
    src/helper/KeyToLabel.cpp
    src/Menu.cpp
    src/Menu/ProcessInputMenu.cpp
    src/Menu/IntroScreen/IntroScreen.cpp
    src/Menu/MainMenu/MainMenu.cpp
    src/Menu/LobbyMenu/LobbyMenu.cpp
    src/Menu/SettingsMenu/SettingsMenu.cpp
    src/Menu/LobbyMenu/LobbyMenu.cpp
    src/Menu/SettingsMenu/SettingsMenu.cpp
    src/Menu/ProfileMenu/ProfileMenu.cpp
    src/LobbyRoomState.cpp
    src/PlayingState.cpp
    src/infomode/InfoMode.cpp
    src/infomode/InfoCategory.cpp
    src/infomode/categories/CpuRamCategory.cpp
    src/infomode/categories/GameCategory.cpp
    src/infomode/categories/DeviceCategory.cpp
    src/infomode/categories/NetworkCategory.cpp
    src/ParallaxBackground.cpp
    src/Overlay.cpp
    src/Client.cpp
    src/systems/NetworkSendSystem.cpp
    src/systems/NetworkReceiveSystem.cpp
)

target_include_directories(client PRIVATE
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(client PRIVATE
    engineCore
    common
    network
    SDL2::SDL2main
    SDL2::SDL2
)

# Ensure SDL2main handles the main/WinMain conversion on Windows
if(WIN32)
    set_target_properties(client PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

add_subdirectory(modules)
