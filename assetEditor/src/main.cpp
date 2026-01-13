// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                         R-TYPE ASSET EDITOR                               ║
// ║                Simple ImGui JSON Editor for Game Config                   ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;
namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// Global State
// ─────────────────────────────────────────────────────────────────────────────
struct EditorState {
    std::string configPath = CONFIG_PATH;
    std::vector<std::string> jsonFiles;
    std::string currentFile;
    json currentJson;
    bool modified = false;
    std::string statusMessage;
    float statusTimer = 0.0f;
};

static EditorState g_state;

// ─────────────────────────────────────────────────────────────────────────────
// File Operations
// ─────────────────────────────────────────────────────────────────────────────
void RefreshFileList() {
    g_state.jsonFiles.clear();
    try {
        for (const auto& entry : fs::directory_iterator(g_state.configPath)) {
            if (entry.path().extension() == ".json") {
                g_state.jsonFiles.push_back(entry.path().filename().string());
            }
        }
        std::sort(g_state.jsonFiles.begin(), g_state.jsonFiles.end());
    } catch (const std::exception& e) {
        g_state.statusMessage = std::string("Error scanning directory: ") + e.what();
        g_state.statusTimer = 3.0f;
    }
}

bool LoadFile(const std::string& filename) {
    std::string fullPath = g_state.configPath + "/" + filename;
    try {
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            g_state.statusMessage = "Failed to open: " + filename;
            g_state.statusTimer = 3.0f;
            return false;
        }
        g_state.currentJson = json::parse(file);
        g_state.currentFile = filename;
        g_state.modified = false;
        g_state.statusMessage = "Loaded: " + filename;
        g_state.statusTimer = 2.0f;
        return true;
    } catch (const std::exception& e) {
        g_state.statusMessage = std::string("Parse error: ") + e.what();
        g_state.statusTimer = 3.0f;
        return false;
    }
}

bool SaveFile() {
    if (g_state.currentFile.empty()) return false;
    
    std::string fullPath = g_state.configPath + "/" + g_state.currentFile;
    try {
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            g_state.statusMessage = "Failed to save: " + g_state.currentFile;
            g_state.statusTimer = 3.0f;
            return false;
        }
        file << g_state.currentJson.dump(2);
        g_state.modified = false;
        g_state.statusMessage = "Saved: " + g_state.currentFile;
        g_state.statusTimer = 2.0f;
        return true;
    } catch (const std::exception& e) {
        g_state.statusMessage = std::string("Save error: ") + e.what();
        g_state.statusTimer = 3.0f;
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Smart Label Detection
// ─────────────────────────────────────────────────────────────────────────────
std::string GetItemLabel(const json& item, size_t index) {
    // Try common identifier fields
    static const std::vector<std::string> labelFields = {"name", "id", "title", "label", "key"};
    
    for (const auto& field : labelFields) {
        if (item.contains(field) && item[field].is_string()) {
            return item[field].get<std::string>();
        }
    }
    return "Item " + std::to_string(index);
}

// ─────────────────────────────────────────────────────────────────────────────
// JSON Value Editor Widgets
// ─────────────────────────────────────────────────────────────────────────────
void EditJsonValue(const std::string& key, json& value) {
    ImGui::PushID(key.c_str());
    
    if (value.is_boolean()) {
        bool b = value.get<bool>();
        if (ImGui::Checkbox(key.c_str(), &b)) {
            value = b;
            g_state.modified = true;
        }
    }
    else if (value.is_number_integer()) {
        int i = value.get<int>();
        if (ImGui::InputInt(key.c_str(), &i)) {
            value = i;
            g_state.modified = true;
        }
    }
    else if (value.is_number_float()) {
        float f = value.get<float>();
        if (ImGui::DragFloat(key.c_str(), &f, 0.1f)) {
            value = f;
            g_state.modified = true;
        }
    }
    else if (value.is_string()) {
        std::string s = value.get<std::string>();
        char buffer[256];
        strncpy(buffer, s.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        if (ImGui::InputText(key.c_str(), buffer, sizeof(buffer))) {
            value = std::string(buffer);
            g_state.modified = true;
        }
    }
    
    ImGui::PopID();
}

// ─────────────────────────────────────────────────────────────────────────────
// Render a nested JSON object (for sub-objects like sprite, transform, etc.)
// ─────────────────────────────────────────────────────────────────────────────
void RenderJsonObject(const std::string& label, json& obj) {
    if (!obj.is_object()) return;
    
    if (ImGui::TreeNode(label.c_str())) {
        for (auto& [key, value] : obj.items()) {
            if (value.is_object()) {
                RenderJsonObject(key, value);
            } else if (value.is_array()) {
                // Simple array display
                if (ImGui::TreeNode(key.c_str())) {
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string itemKey = "[" + std::to_string(i) + "]";
                        if (value[i].is_object()) {
                            RenderJsonObject(itemKey, value[i]);
                        } else {
                            EditJsonValue(itemKey, value[i]);
                        }
                    }
                    ImGui::TreePop();
                }
            } else {
                EditJsonValue(key, value);
            }
        }
        ImGui::TreePop();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render a single list item with all its properties
// ─────────────────────────────────────────────────────────────────────────────
void RenderListItem(json& item, size_t index) {
    std::string label = GetItemLabel(item, index);
    
    ImGui::PushID(static_cast<int>(index));
    
    if (ImGui::CollapsingHeader(label.c_str())) {
        ImGui::Indent(10.0f);
        
        for (auto& [key, value] : item.items()) {
            if (value.is_object()) {
                RenderJsonObject(key, value);
            } else if (value.is_array()) {
                if (ImGui::TreeNode(key.c_str())) {
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string itemKey = "[" + std::to_string(i) + "]";
                        if (value[i].is_object()) {
                            RenderJsonObject(itemKey, value[i]);
                        } else {
                            EditJsonValue(itemKey, value[i]);
                        }
                    }
                    ImGui::TreePop();
                }
            } else {
                EditJsonValue(key, value);
            }
        }
        
        ImGui::Unindent(10.0f);
    }
    
    ImGui::PopID();
}

// ─────────────────────────────────────────────────────────────────────────────
// Detect main array in JSON root and render directly
// ─────────────────────────────────────────────────────────────────────────────
void RenderJsonEditor() {
    if (g_state.currentJson.is_null()) {
        ImGui::TextDisabled("No file loaded. Select a JSON file from the left panel.");
        return;
    }
    
    // Smart detection: if root is object with a single array, dive into it
    std::string mainArrayKey;
    json* mainArray = nullptr;
    
    if (g_state.currentJson.is_object()) {
        for (auto& [key, value] : g_state.currentJson.items()) {
            if (value.is_array()) {
                mainArrayKey = key;
                mainArray = &value;
                break; // Take first array found
            }
        }
    }
    
    if (mainArray && !mainArray->empty()) {
        // Direct list display mode
        ImGui::Text("Editing: %s (%zu items)", mainArrayKey.c_str(), mainArray->size());
        ImGui::Separator();
        
        for (size_t i = 0; i < mainArray->size(); ++i) {
            RenderListItem((*mainArray)[i], i);
        }
    } else if (g_state.currentJson.is_object()) {
        // Fallback: render as generic object
        ImGui::Text("Object Editor");
        ImGui::Separator();
        
        for (auto& [key, value] : g_state.currentJson.items()) {
            if (value.is_object()) {
                RenderJsonObject(key, value);
            } else if (value.is_array()) {
                if (ImGui::TreeNode(key.c_str())) {
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string itemKey = "[" + std::to_string(i) + "]";
                        if (value[i].is_object()) {
                            RenderJsonObject(itemKey, value[i]);
                        } else {
                            EditJsonValue(itemKey, value[i]);
                        }
                    }
                    ImGui::TreePop();
                }
            } else {
                EditJsonValue(key, value);
            }
        }
    } else if (g_state.currentJson.is_array()) {
        // Direct array at root
        ImGui::Text("Array Editor (%zu items)", g_state.currentJson.size());
        ImGui::Separator();
        
        for (size_t i = 0; i < g_state.currentJson.size(); ++i) {
            RenderListItem(g_state.currentJson[i], i);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Main UI Render
// ─────────────────────────────────────────────────────────────────────────────
void RenderUI() {
    // Full window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_MenuBar;
    
    ImGui::Begin("Asset Editor", nullptr, windowFlags);
    
    // ─────────────────────────────────────────────────────────────────────────
    // Menu Bar
    // ─────────────────────────────────────────────────────────────────────────
    if (ImGui::BeginMenuBar()) {
        // Save Button
        if (!g_state.currentFile.empty()) {
            if (g_state.modified) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            }
            if (ImGui::Button("Save")) {
                SaveFile();
            }
            if (g_state.modified) {
                ImGui::PopStyleColor();
            }
            
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            
            if (g_state.modified) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "%s *", g_state.currentFile.c_str());
            } else {
                ImGui::Text("%s", g_state.currentFile.c_str());
            }
        }
        
        // Status message
        if (g_state.statusTimer > 0.0f) {
            ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(g_state.statusMessage.c_str()).x - 20);
            ImGui::TextDisabled("%s", g_state.statusMessage.c_str());
        }
        
        ImGui::EndMenuBar();
    }
    
    // ─────────────────────────────────────────────────────────────────────────
    // Main Layout: Left Panel (Files) + Right Panel (Editor)
    // ─────────────────────────────────────────────────────────────────────────
    float panelWidth = ImGui::GetContentRegionAvail().x * 0.20f;
    
    // Left Panel: File List
    ImGui::BeginChild("FileList", ImVec2(panelWidth, 0), true);
    {
        ImGui::Text("Config Files");
        ImGui::Separator();
        
        if (ImGui::Button("Refresh", ImVec2(-1, 0))) {
            RefreshFileList();
        }
        
        ImGui::Spacing();
        
        for (const auto& file : g_state.jsonFiles) {
            bool isSelected = (file == g_state.currentFile);
            if (ImGui::Selectable(file.c_str(), isSelected)) {
                if (file != g_state.currentFile) {
                    LoadFile(file);
                }
            }
        }
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Right Panel: JSON Editor
    ImGui::BeginChild("Editor", ImVec2(0, 0), true);
    {
        RenderJsonEditor();
    }
    ImGui::EndChild();
    
    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
// Main Entry Point
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create window (without ALLOW_HIGHDPI to avoid mouse offset issues)
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
    
    // Create renderer with logical size matching window size
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Set logical size to match window size (prevents scaling issues)
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    SDL_RenderSetLogicalSize(renderer, windowW, windowH);
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Standard ImGui style (no fancy theming)
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    
    // Ensure display size is correct
    io.DisplaySize = ImVec2(static_cast<float>(windowW), static_cast<float>(windowH));
    
    // Initial file list scan
    RefreshFileList();
    
    // Main loop
    bool running = true;
    while (running) {
        // Poll events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_WINDOWEVENT && 
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
            
            // Handle window resize
            if (event.type == SDL_WINDOWEVENT && 
                event.window.event == SDL_WINDOWEVENT_RESIZED) {
                int newW = event.window.data1;
                int newH = event.window.data2;
                SDL_RenderSetLogicalSize(renderer, newW, newH);
            }
            
            // Keyboard shortcuts
            if (event.type == SDL_KEYDOWN) {
                if ((event.key.keysym.mod & KMOD_GUI) || (event.key.keysym.mod & KMOD_CTRL)) {
                    if (event.key.keysym.sym == SDLK_s) {
                        SaveFile();
                    }
                }
            }
        }
        
        // Update status timer
        if (g_state.statusTimer > 0.0f) {
            g_state.statusTimer -= io.DeltaTime;
        }
        
        // Start ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        // Render UI
        RenderUI();
        
        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
    
    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
