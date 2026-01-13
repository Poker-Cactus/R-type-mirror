cmake_minimum_required(VERSION 3.15)
project(AssetEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─────────────────────────────────────────────────────────────────────────────
# ImGui sources (vendored)
# ─────────────────────────────────────────────────────────────────────────────
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

add_library(imgui_asseteditor STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_asseteditor PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui_asseteditor PUBLIC SDL2::SDL2)

# ─────────────────────────────────────────────────────────────────────────────
# Asset Editor executable
# ─────────────────────────────────────────────────────────────────────────────
add_executable(assetEditor
    src/main.cpp
)

target_include_directories(assetEditor PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(assetEditor PRIVATE
    imgui_asseteditor
    SDL2::SDL2
    SDL2::SDL2main
    nlohmann_json::nlohmann_json
)

# Copy config path reference
target_compile_definitions(assetEditor PRIVATE
    CONFIG_PATH="${CMAKE_SOURCE_DIR}/server/config"
)
