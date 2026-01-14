/**
 * @file main.cpp
 * @brief R-Type Asset Editor - Main entry point
 *
 * Standalone ImGui-based editor for game configuration files and sprites.
 * Features JSON editing, sprite management, macOS-styled UI, and bundled Inter font.
 */

#include "EditorState.hpp"
#include "JsonEditor.hpp"
#include "MainMenu.hpp"
#include "SpriteEditor.hpp"
#include "Style.hpp"

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <filesystem>
#include <iostream>
#include <memory>

using namespace AssetEditor;

// ═══════════════════════════════════════════════════════════════════════════
// RAII Wrappers for SDL
// ═══════════════════════════════════════════════════════════════════════════

namespace {

/**
 * @brief RAII wrapper for SDL initialization
 */
class SDLInitializer {
public:
    explicit SDLInitializer(Uint32 flags) : m_initialized(false) {
        if (SDL_Init(flags) == 0) {
            m_initialized = true;
        }
    }

    ~SDLInitializer() {
        if (m_initialized) {
            SDL_Quit();
        }
    }

    [[nodiscard]] bool IsValid() const noexcept { return m_initialized; }

    // Non-copyable, non-movable
    SDLInitializer(const SDLInitializer&) = delete;
    SDLInitializer& operator=(const SDLInitializer&) = delete;

private:
    bool m_initialized;
};

/**
 * @brief Custom deleter for SDL_Window
 */
struct WindowDeleter {
    void operator()(SDL_Window* window) const noexcept {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
};

/**
 * @brief Custom deleter for SDL_Renderer
 */
struct RendererDeleter {
    void operator()(SDL_Renderer* renderer) const noexcept {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
    }
};

using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;

} // namespace

// ═══════════════════════════════════════════════════════════════════════════
// UI Rendering
// ═══════════════════════════════════════════════════════════════════════════

namespace {

/**
 * @brief Render the appropriate UI based on current editor mode
 */
void RenderUI() {
    switch (g_state.mode) {
        case EditorMode::MainMenu:
            RenderMainMenu();
            break;
        case EditorMode::JsonEditor:
            RenderJsonEditorUI();
            break;
        case EditorMode::SpriteEditor:
            RenderSpriteEditorUI();
            break;
    }
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════
// Main Entry Point
// ═══════════════════════════════════════════════════════════════════════════

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Initialize SDL with RAII
    SDLInitializer sdl(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    if (!sdl.IsValid()) {
        std::cerr << "[AssetEditor] SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window with RAII
    WindowPtr window(SDL_CreateWindow(
        "R-Type Asset Editor",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_RESIZABLE
    ));

    if (!window) {
        std::cerr << "[AssetEditor] SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create renderer with RAII
    RendererPtr renderer(SDL_CreateRenderer(
        window.get(), -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
    ));

    if (!renderer) {
        std::cerr << "[AssetEditor] SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Set logical size to match window size (prevents High DPI mouse offset)
    int windowW = 0;
    int windowH = 0;
    SDL_GetWindowSize(window.get(), &windowW, &windowH);
    SDL_RenderSetLogicalSize(renderer.get(), windowW, windowH);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    LoadFont(io);
    ApplyMacOSStyle();

    // Setup ImGui backends
    ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer2_Init(renderer.get());

    io.DisplaySize = ImVec2(static_cast<float>(windowW), static_cast<float>(windowH));

    // Setup sprite renderer (non-owning pointer - renderer lifetime managed above)
    SetSpriteRenderer(renderer.get());

    // Initial setup
    RefreshFileList();

    // Main loop
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) {
                running = false;
            }

            // Window resize
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_RESIZED) {
                const int newW = event.window.data1;
                const int newH = event.window.data2;
                SDL_RenderSetLogicalSize(renderer.get(), newW, newH);
            }

            // Drag and drop file
            if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                if (droppedFile) {
                    if (g_state.mode == EditorMode::SpriteEditor) {
                        if (IsImportOverlayActive()) {
                            (void)HandleImportDroppedFile(droppedFile);
                        } else if (IsSupportedSpriteExtension(
                            std::filesystem::path(droppedFile).extension().string())) {
                            (void)ImportSpriteFile(droppedFile);
                        }
                    }
                    SDL_free(droppedFile);
                }
            }

            // Keyboard shortcuts (Cmd+S / Ctrl+S to save)
            if (event.type == SDL_KEYDOWN) {
                if ((event.key.keysym.mod & KMOD_GUI) || (event.key.keysym.mod & KMOD_CTRL)) {
                    if (event.key.keysym.sym == SDLK_s) {
                        (void)SaveFile();
                    }
                }
            }
        }

        // Start new ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Render UI
        RenderUI();

        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer.get(), 28, 28, 30, 255);  // macOS Gray6
        SDL_RenderClear(renderer.get());
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer.get());
    }

    // Cleanup
    CleanupSpriteTextures();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
