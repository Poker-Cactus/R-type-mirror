project(client)

add_executable(client
    src/main.cpp
    src/Game.cpp
    src/Menu.cpp
    src/Menu/RenderMenus.cpp
    src/Menu/ProcessInputMenu.cpp
)

target_include_directories(client PRIVATE include)

target_link_libraries(client PRIVATE 
    engine_core 
    common 
    SDL2::SDL2
)

add_subdirectory(modules)