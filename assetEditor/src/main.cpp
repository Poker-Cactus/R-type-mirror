/**
 * @file main.cpp
 * @brief R-Type Asset Editor - Main entry point
 * 
 * Standalone ImGui-based editor for game configuration files and sprites.
 * Features JSON editing, sprite management, macOS-styled UI, and bundled Inter font.
 */

#include "EditorState.h"
#include "JsonEditor.h"
#include "MainMenu.h"
#include "SpriteEditor.h"
#include "Style.h"

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <iostream>
#include <filesystem>

using namespace AssetEditor;

/**
 * @brief Render the appropriate UI based on current editor mode
 */
static void RenderUI() {
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

/**
 * @brief Main application entry point
 */
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "R-Type Asset Editor",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Set logical size to match window size (prevents High DPI mouse offset)
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    SDL_RenderSetLogicalSize(renderer, windowW, windowH);
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    LoadFont(io);
    ApplyMacOSStyle();
    
    // Setup backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    
    io.DisplaySize = ImVec2(static_cast<float>(windowW), static_cast<float>(windowH));
    
    // Setup sprite renderer
    SetSpriteRenderer(renderer);
    
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
                int newW = event.window.data1;
                int newH = event.window.data2;
                SDL_RenderSetLogicalSize(renderer, newW, newH);
            }
            
            // Drag and drop file
            if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                if (droppedFile) {
                    // Check if we're in sprite editor mode
                    if (g_state.mode == EditorMode::SpriteEditor) {
                        // First check if import overlay is active
                        if (IsImportOverlayActive()) {
                            HandleImportDroppedFile(droppedFile);
                        } else if (IsSupportedSpriteExtension(std::filesystem::path(droppedFile).extension().string())) {
                            ImportSpriteFile(droppedFile);
                        }
                    }
                    SDL_free(droppedFile);
                }
            }
            
            // Keyboard shortcuts (Cmd+S / Ctrl+S to save)
            if (event.type == SDL_KEYDOWN) {
                if ((event.key.keysym.mod & KMOD_GUI) || (event.key.keysym.mod & KMOD_CTRL)) {
                    if (event.key.keysym.sym == SDLK_s) {
                        SaveFile();
                    }
                }
            }
        }
        
        // Start frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        RenderUI();
        
        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 41, 41, 48, 255); // macOS dark background
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
    
    // Cleanup
    CleanupSpriteTextures();
    
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
