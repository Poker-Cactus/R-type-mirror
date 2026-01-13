/**
 * @file SpriteEditor.cpp
 * @brief Sprite editor functionality implementation
 * 
 * Uses authentic macOS system colors from Apple HIG.
 */

#include "SpriteEditor.h"
#include "EditorState.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include <SDL_image.h>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace AssetEditor {

// macOS System Colors (Dark Mode)
namespace {
    const ImVec4 MacBlue   = ImVec4(0.00f, 0.57f, 1.00f, 1.00f);  // RGB(0, 145, 255)
    const ImVec4 MacGreen  = ImVec4(0.19f, 0.82f, 0.35f, 1.00f);  // RGB(48, 209, 88)
}

// Sprite cache
static std::vector<SpriteInfo> s_spriteCache;
static SDL_Renderer* s_renderer = nullptr;

// Layer structure
struct Layer {
    std::string name;
    SDL_Surface* surface = nullptr;
    bool visible = true;
    int zOrder = 0;
};

// Pixel editor state
static float s_zoomLevel = 1.0f;
static bool s_showGrid = true;
static ImVec4 s_currentColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
static int s_selectedTool = 0; // 0=Pencil, 1=Eraser, 2=Eyedropper
static int s_brushSize = 1; // Brush size in pixels
static SDL_Surface* s_editingSurface = nullptr; // Surface for editing (deprecated, use layers)
static bool s_isDrawing = false;
static bool s_spriteModified = false;
static char s_renameBuffer[256] = "";

// Notification system
static std::string s_notificationMessage = "";
static float s_notificationTimer = 0.0f;
static const float NOTIFICATION_DURATION = 2.0f;

// Layer system
static std::vector<Layer> s_layers;
static int s_activeLayerIndex = 0;
static int s_layerCounter = 0;
static int s_renamingLayerIndex = -1; // -1 = not renaming, >= 0 = renaming layer at index
static char s_layerRenameBuffer[64] = "";

// Clipboard for copy/paste
static SDL_Surface* s_clipboardSurface = nullptr;

// Canvas dimensions (can grow when importing larger images)
static int s_canvasWidth = 0;
static int s_canvasHeight = 0;

// Undo/Redo system - only for pixel changes, not layer operations
static std::vector<SDL_Surface*> s_undoStack;
static std::vector<SDL_Surface*> s_redoStack;
static const int MAX_UNDO_HISTORY = 50;

// Import Image overlay state
static bool s_showImportOverlay = false;
static std::string s_importFilePath = "";
static SDL_Surface* s_importPreviewSurface = nullptr;
static int s_importOffsetX = 0;
static int s_importOffsetY = 0;
static float s_importScale = 1.0f;
static float s_importOpacity = 1.0f;
static bool s_importAsNewLayer = true;

// Helper: Free import preview surface
static void FreeImportPreview() {
    if (s_importPreviewSurface) {
        SDL_FreeSurface(s_importPreviewSurface);
        s_importPreviewSurface = nullptr;
    }
    s_importFilePath = "";
    s_importOffsetX = 0;
    s_importOffsetY = 0;
    s_importScale = 1.0f;
    s_importOpacity = 1.0f;
    s_importAsNewLayer = true;
}

// Helper: Load image for import overlay
static bool LoadImportPreview(const std::string& filePath) {
    FreeImportPreview();
    
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        return false;
    }
    
    // Convert to RGBA32
    s_importPreviewSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    
    if (!s_importPreviewSurface) {
        return false;
    }
    
    s_importFilePath = filePath;
    return true;
}

// Forward declarations for functions used by ApplyImportedImage
static void PushUndoState();
static void UpdateEditingSurface(SpriteInfo* info);
static void UpdateTextureFromSurface(SpriteInfo* info);
static void FreeUndoStack();
static void FreeRedoStack();
static void ResizeCanvas(int newWidth, int newHeight, SpriteInfo* info);

// Helper: Apply imported image to layer
static void ApplyImportedImage(SpriteInfo* info) {
    if (!s_importPreviewSurface || s_layers.empty()) return;
    
    // Scale the import surface
    int scaledW = static_cast<int>(s_importPreviewSurface->w * s_importScale);
    int scaledH = static_cast<int>(s_importPreviewSurface->h * s_importScale);
    
    // Check if we need to expand canvas to fit the imported image
    int requiredW = s_importOffsetX + scaledW;
    int requiredH = s_importOffsetY + scaledH;
    
    // Also handle negative offsets by expanding on the other side
    int expandLeft = (s_importOffsetX < 0) ? -s_importOffsetX : 0;
    int expandTop = (s_importOffsetY < 0) ? -s_importOffsetY : 0;
    
    int newCanvasW = std::max(s_canvasWidth + expandLeft, requiredW);
    int newCanvasH = std::max(s_canvasHeight + expandTop, requiredH);
    
    // Resize canvas if needed (this will expand all existing layers)
    if (newCanvasW > s_canvasWidth || newCanvasH > s_canvasHeight) {
        ResizeCanvas(newCanvasW, newCanvasH, info);
    }
    
    // Create scaled surface
    SDL_Surface* scaledSurface = SDL_CreateRGBSurfaceWithFormat(0, scaledW, scaledH, 32, SDL_PIXELFORMAT_RGBA32);
    if (!scaledSurface) return;
    
    // Scale with SDL_BlitScaled
    SDL_BlitScaled(s_importPreviewSurface, nullptr, scaledSurface, nullptr);
    
    // Apply opacity to scaled surface
    if (s_importOpacity < 1.0f) {
        SDL_SetSurfaceAlphaMod(scaledSurface, static_cast<Uint8>(s_importOpacity * 255));
        SDL_SetSurfaceBlendMode(scaledSurface, SDL_BLENDMODE_BLEND);
    }
    
    // Adjust offset if canvas was expanded on left/top
    int adjustedOffsetX = s_importOffsetX + expandLeft;
    int adjustedOffsetY = s_importOffsetY + expandTop;
    
    if (s_importAsNewLayer) {
        // Create new layer with the imported image
        Layer newLayer;
        newLayer.name = "Import " + std::to_string(s_layerCounter++);
        newLayer.surface = SDL_CreateRGBSurfaceWithFormat(0, s_canvasWidth, s_canvasHeight, 32, SDL_PIXELFORMAT_RGBA32);
        
        // Clear to transparent
        SDL_FillRect(newLayer.surface, nullptr, SDL_MapRGBA(newLayer.surface->format, 0, 0, 0, 0));
        
        // Blit scaled image at offset
        SDL_Rect dstRect = {adjustedOffsetX, adjustedOffsetY, scaledW, scaledH};
        SDL_BlitSurface(scaledSurface, nullptr, newLayer.surface, &dstRect);
        
        newLayer.visible = true;
        newLayer.zOrder = s_layers.size();
        s_layers.push_back(newLayer);
        s_activeLayerIndex = s_layers.size() - 1;
    } else {
        // Merge onto active layer
        if (s_activeLayerIndex < static_cast<int>(s_layers.size()) && s_layers[s_activeLayerIndex].surface) {
            PushUndoState();
            
            SDL_Rect dstRect = {adjustedOffsetX, adjustedOffsetY, scaledW, scaledH};
            SDL_BlitSurface(scaledSurface, nullptr, s_layers[s_activeLayerIndex].surface, &dstRect);
            s_spriteModified = true;
        }
    }
    
    SDL_FreeSurface(scaledSurface);
    
    // Update composite
    if (info) {
        UpdateEditingSurface(info);
        UpdateTextureFromSurface(info);
    }
    
    // Clear undo/redo for new layer
    if (s_importAsNewLayer) {
        FreeUndoStack();
        FreeRedoStack();
    }
    
    FreeImportPreview();
    s_showImportOverlay = false;
    s_spriteModified = true;
}

// Helper: Clone a surface
static SDL_Surface* CloneSurface(SDL_Surface* src) {
    if (!src) return nullptr;
    return SDL_ConvertSurface(src, src->format, 0);
}

// Helper: Free undo/redo surfaces
static void FreeUndoStack() {
    for (auto* surface : s_undoStack) {
        if (surface) SDL_FreeSurface(surface);
    }
    s_undoStack.clear();
}

static void FreeRedoStack() {
    for (auto* surface : s_redoStack) {
        if (surface) SDL_FreeSurface(surface);
    }
    s_redoStack.clear();
}

// Helper: Free layer surfaces
static void FreeLayers(std::vector<Layer>& layers) {
    for (auto& layer : layers) {
        if (layer.surface) {
            SDL_FreeSurface(layer.surface);
            layer.surface = nullptr;
        }
    }
}

// Helper: Free clipboard
static void FreeClipboard() {
    if (s_clipboardSurface) {
        SDL_FreeSurface(s_clipboardSurface);
        s_clipboardSurface = nullptr;
    }
}

// Helper: Copy active layer to clipboard
static void CopyLayerToClipboard() {
    if (s_activeLayerIndex >= static_cast<int>(s_layers.size())) return;
    if (!s_layers[s_activeLayerIndex].surface) return;
    
    FreeClipboard();
    s_clipboardSurface = CloneSurface(s_layers[s_activeLayerIndex].surface);
}

// Helper: Resize all layers to new dimensions
static void ResizeCanvas(int newWidth, int newHeight, SpriteInfo* info);

// Helper: Paste clipboard as new layer
static void PasteFromClipboard(SpriteInfo* info) {
    if (!s_clipboardSurface || s_layers.empty()) return;
    
    // Check if we need to expand canvas
    int clipW = s_clipboardSurface->w;
    int clipH = s_clipboardSurface->h;
    
    if (clipW > s_canvasWidth || clipH > s_canvasHeight) {
        int newW = std::max(s_canvasWidth, clipW);
        int newH = std::max(s_canvasHeight, clipH);
        ResizeCanvas(newW, newH, info);
    }
    
    // Create new layer from clipboard
    Layer newLayer;
    newLayer.name = "Paste " + std::to_string(s_layerCounter++);
    newLayer.surface = SDL_CreateRGBSurfaceWithFormat(0, s_canvasWidth, s_canvasHeight, 32, SDL_PIXELFORMAT_RGBA32);
    
    // Clear to transparent
    SDL_FillRect(newLayer.surface, nullptr, SDL_MapRGBA(newLayer.surface->format, 0, 0, 0, 0));
    
    // Blit clipboard content (centered)
    SDL_Rect dstRect = {0, 0, clipW, clipH};
    SDL_BlitSurface(s_clipboardSurface, nullptr, newLayer.surface, &dstRect);
    
    newLayer.visible = true;
    newLayer.zOrder = s_layers.size();
    s_layers.push_back(newLayer);
    s_activeLayerIndex = s_layers.size() - 1;
    
    // Update
    if (info) {
        UpdateEditingSurface(info);
        UpdateTextureFromSurface(info);
    }
    
    FreeUndoStack();
    FreeRedoStack();
    s_spriteModified = true;
}

// Helper: Resize canvas (expand all layers)
static void ResizeCanvas(int newWidth, int newHeight, SpriteInfo* info) {
    if (newWidth <= 0 || newHeight <= 0) return;
    if (newWidth == s_canvasWidth && newHeight == s_canvasHeight) return;
    
    // Resize each layer
    for (auto& layer : s_layers) {
        if (!layer.surface) continue;
        
        SDL_Surface* newSurface = SDL_CreateRGBSurfaceWithFormat(0, newWidth, newHeight, 32, SDL_PIXELFORMAT_RGBA32);
        if (!newSurface) continue;
        
        // Clear to transparent
        SDL_FillRect(newSurface, nullptr, SDL_MapRGBA(newSurface->format, 0, 0, 0, 0));
        
        // Copy old content (top-left aligned)
        SDL_BlitSurface(layer.surface, nullptr, newSurface, nullptr);
        
        // Replace surface
        SDL_FreeSurface(layer.surface);
        layer.surface = newSurface;
    }
    
    s_canvasWidth = newWidth;
    s_canvasHeight = newHeight;
    
    // Update sprite info dimensions
    if (info) {
        info->width = newWidth;
        info->height = newHeight;
        UpdateEditingSurface(info);
        UpdateTextureFromSurface(info);
    }
    
    s_spriteModified = true;
}

// Forward declarations for helper functions
static void UpdateEditingSurface(SpriteInfo* info);
static void UpdateTextureFromSurface(SpriteInfo* info);

// Initialize layers from sprite
static void InitializeLayers(SpriteInfo* info) {
    // Clear existing layers
    FreeLayers(s_layers);
    s_layers.clear();
    s_activeLayerIndex = 0;
    s_layerCounter = 0;
    s_renamingLayerIndex = -1;
    
    // Load sprite as first layer
    SDL_Surface* surface = IMG_Load(info->fullPath.c_str());
    if (!surface) return;
    
    // Convert to RGBA
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    
    if (converted) {
        // Set canvas dimensions
        s_canvasWidth = converted->w;
        s_canvasHeight = converted->h;
        
        Layer baseLayer;
        baseLayer.name = "Layer 0";
        baseLayer.surface = converted;
        baseLayer.visible = true;
        baseLayer.zOrder = 0;
        s_layers.push_back(baseLayer);
        s_layerCounter++;
    }
    
    // Clear undo/redo stacks
    FreeUndoStack();
    FreeRedoStack();
}

// Save state to undo stack (only active layer)
static void PushUndoState() {
    if (s_activeLayerIndex >= static_cast<int>(s_layers.size()) || !s_layers[s_activeLayerIndex].surface) {
        return;
    }
    
    // Clone active layer surface
    SDL_Surface* state = CloneSurface(s_layers[s_activeLayerIndex].surface);
    if (!state) return;
    
    s_undoStack.push_back(state);
    
    // Limit undo history
    if (s_undoStack.size() > MAX_UNDO_HISTORY) {
        if (s_undoStack.front()) {
            SDL_FreeSurface(s_undoStack.front());
        }
        s_undoStack.erase(s_undoStack.begin());
    }
    
    // Clear redo stack when new action is performed
    FreeRedoStack();
    
    s_spriteModified = true;
}

// Undo
static void Undo() {
    if (s_undoStack.empty() || s_activeLayerIndex >= static_cast<int>(s_layers.size())) return;
    
    // Save current state to redo stack
    SDL_Surface* currentState = CloneSurface(s_layers[s_activeLayerIndex].surface);
    if (currentState) {
        s_redoStack.push_back(currentState);
    }
    
    // Restore previous state
    SDL_Surface* previousState = s_undoStack.back();
    s_undoStack.pop_back();
    
    if (s_layers[s_activeLayerIndex].surface) {
        SDL_FreeSurface(s_layers[s_activeLayerIndex].surface);
    }
    s_layers[s_activeLayerIndex].surface = previousState;
    
    // Update composite and texture
    for (auto& info : s_spriteCache) {
        if (info.filename == g_state.selectedSprite) {
            UpdateEditingSurface(&info);
            UpdateTextureFromSurface(&info);
            break;
        }
    }
}

// Redo
static void Redo() {
    if (s_redoStack.empty() || s_activeLayerIndex >= static_cast<int>(s_layers.size())) return;
    
    // Save current state to undo stack
    SDL_Surface* currentState = CloneSurface(s_layers[s_activeLayerIndex].surface);
    if (currentState) {
        s_undoStack.push_back(currentState);
    }
    
    // Restore next state
    SDL_Surface* nextState = s_redoStack.back();
    s_redoStack.pop_back();
    
    if (s_layers[s_activeLayerIndex].surface) {
        SDL_FreeSurface(s_layers[s_activeLayerIndex].surface);
    }
    s_layers[s_activeLayerIndex].surface = nextState;
    
    // Update composite and texture
    for (auto& info : s_spriteCache) {
        if (info.filename == g_state.selectedSprite) {
            UpdateEditingSurface(&info);
            UpdateTextureFromSurface(&info);
            break;
        }
    }
}

// Composite all visible layers into single surface
static SDL_Surface* CompositeLayersToSurface() {
    if (s_layers.empty()) return nullptr;
    
    // Use canvas dimensions (handles expanded canvas)
    int width = s_canvasWidth > 0 ? s_canvasWidth : s_layers[0].surface->w;
    int height = s_canvasHeight > 0 ? s_canvasHeight : s_layers[0].surface->h;
    
    // Create result surface
    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!result) return nullptr;
    
    // Clear to transparent
    SDL_FillRect(result, nullptr, SDL_MapRGBA(result->format, 0, 0, 0, 0));
    
    // Sort layers by z-order
    std::vector<int> layerIndices;
    for (int i = 0; i < static_cast<int>(s_layers.size()); i++) {
        layerIndices.push_back(i);
    }
    std::sort(layerIndices.begin(), layerIndices.end(), [](int a, int b) {
        return s_layers[a].zOrder < s_layers[b].zOrder;
    });
    
    // Blend layers
    for (int idx : layerIndices) {
        const auto& layer = s_layers[idx];
        if (layer.visible && layer.surface) {
            SDL_BlitSurface(layer.surface, nullptr, result, nullptr);
        }
    }
    
    return result;
}

void UpdateEditingSurface(SpriteInfo* info) {
    if (!info || !s_renderer) return;
    
    if (s_editingSurface) {
        SDL_FreeSurface(s_editingSurface);
        s_editingSurface = nullptr;
    }
    
    // Composite layers
    s_editingSurface = CompositeLayersToSurface();
}

void UpdateTextureFromSurface(SpriteInfo* info) {
    if (!info || !s_editingSurface || !s_renderer) return;
    
    // Destroy old texture
    if (info->textureId) {
        SDL_DestroyTexture(static_cast<SDL_Texture*>(info->textureId));
        info->textureId = nullptr;
    }
    
    // Create new texture from modified surface
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(s_renderer, s_editingSurface);
    if (newTexture) {
        info->textureId = newTexture;
    }
}

void SetSpriteRenderer(SDL_Renderer* renderer) {
    s_renderer = renderer;
}

void CleanupSpriteTextures() {
    for (auto& sprite : s_spriteCache) {
        if (sprite.textureId) {
            SDL_DestroyTexture(static_cast<SDL_Texture*>(sprite.textureId));
            sprite.textureId = nullptr;
        }
    }
}

bool LoadSpriteTexture(SpriteInfo& info) {
    if (!s_renderer) return false;
    if (info.textureId) return true; // Already loaded
    
    SDL_Surface* surface = IMG_Load(info.fullPath.c_str());
    if (!surface) {
        return false;
    }
    
    info.width = surface->w;
    info.height = surface->h;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(s_renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        return false;
    }
    
    info.textureId = texture;
    return true;
}

void RefreshSpriteList() {
    CleanupSpriteTextures();
    s_spriteCache.clear();
    g_state.spriteFiles.clear();
    
    if (!fs::exists(g_state.spritePath)) {
        return;
    }
    
    // Supported sprite extensions
    const std::vector<std::string> validExtensions = {".png", ".gif", ".jpg", ".jpeg", ".bmp"};
    
    for (const auto& entry : fs::directory_iterator(g_state.spritePath)) {
        if (!entry.is_regular_file()) continue;
        
        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        if (std::find(validExtensions.begin(), validExtensions.end(), extension) != validExtensions.end()) {
            SpriteInfo info;
            info.filename = entry.path().filename().string();
            info.fullPath = entry.path().string();
            info.extension = extension;
            info.fileSize = fs::file_size(entry.path());
            
            s_spriteCache.push_back(info);
            g_state.spriteFiles.push_back(info.filename);
        }
    }
    
    // Sort alphabetically
    std::sort(g_state.spriteFiles.begin(), g_state.spriteFiles.end());
    std::sort(s_spriteCache.begin(), s_spriteCache.end(), 
        [](const SpriteInfo& a, const SpriteInfo& b) {
            return a.filename < b.filename;
        });
}

// Supported sprite extensions
static const std::vector<std::string> s_validExtensions = {".png", ".gif", ".jpg", ".jpeg", ".bmp"};

bool IsSupportedSpriteExtension(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(s_validExtensions.begin(), s_validExtensions.end(), ext) != s_validExtensions.end();
}

bool ImportSpriteFile(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        return false;
    }
    
    // Check extension
    std::string extension = fs::path(filePath).extension().string();
    if (!IsSupportedSpriteExtension(extension)) {
        return false;
    }
    
    // Get filename
    std::string filename = fs::path(filePath).filename().string();
    
    // Destination path
    fs::path destPath = fs::path(g_state.spritePath) / filename;
    
    // Check if file already exists
    if (fs::exists(destPath)) {
        // Generate unique name
        std::string baseName = fs::path(filePath).stem().string();
        int counter = 1;
        while (fs::exists(destPath)) {
            filename = baseName + "_" + std::to_string(counter) + extension;
            destPath = fs::path(g_state.spritePath) / filename;
            counter++;
        }
    }
    
    // Copy file
    try {
        fs::copy_file(filePath, destPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to import sprite: " << e.what() << std::endl;
        return false;
    }
    
    // Refresh list and select the new sprite
    RefreshSpriteList();
    SelectSprite(filename);
    
    return true;
}

void SelectSprite(const std::string& filename) {
    g_state.selectedSprite = filename;
    s_spriteModified = false;
    
    // Initialize rename buffer
    strncpy(s_renameBuffer, filename.c_str(), sizeof(s_renameBuffer) - 1);
    s_renameBuffer[sizeof(s_renameBuffer) - 1] = '\0';
    
    // Preload texture for selected sprite
    for (auto& info : s_spriteCache) {
        if (info.filename == filename) {
            LoadSpriteTexture(info);
            InitializeLayers(&info);
            UpdateEditingSurface(&info);
            break;
        }
    }
}

std::string FormatFileSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unitIndex]);
    return std::string(buffer);
}

bool SaveSprite(SpriteInfo* info) {
    if (!info) return false;
    
    // Composite all layers to get final image
    SDL_Surface* compositeSurface = CompositeLayersToSurface();
    if (!compositeSurface) return false;
    
    // Determine save path
    std::string savePath;
    if (strcmp(s_renameBuffer, info->filename.c_str()) != 0) {
        // New filename
        fs::path originalPath(info->fullPath);
        fs::path newPath = originalPath.parent_path() / s_renameBuffer;
        savePath = newPath.string();
    } else {
        // Same filename
        savePath = info->fullPath;
    }
    
    // Save surface to file using SDL_image
    bool success = (IMG_SavePNG(compositeSurface, savePath.c_str()) == 0);
    
    SDL_FreeSurface(compositeSurface);
    
    if (!success) {
        return false;
    }
    
    // Update info if renamed
    if (savePath != info->fullPath) {
        info->filename = s_renameBuffer;
        info->fullPath = savePath;
        g_state.selectedSprite = s_renameBuffer;
        RefreshSpriteList();
    }
    
    s_spriteModified = false;
    return true;
}

void RenderPixelEditor(SpriteInfo* spriteInfo) {
    if (!spriteInfo || !spriteInfo->textureId) {
        ImGui::TextDisabled("No sprite loaded for editing");
        return;
    }
    
    // Handle keyboard shortcuts (Cmd/Ctrl + key)
    ImGuiIO& io = ImGui::GetIO();
    bool cmdOrCtrl = io.KeySuper || io.KeyCtrl;
    
    if (cmdOrCtrl && !io.WantTextInput) {
        // Cmd+C: Copy layer
        if (ImGui::IsKeyPressed(ImGuiKey_C)) {
            CopyLayerToClipboard();
            s_notificationMessage = "Layer copied!";
            s_notificationTimer = NOTIFICATION_DURATION;
        }
        // Cmd+V: Paste layer
        if (ImGui::IsKeyPressed(ImGuiKey_V)) {
            PasteFromClipboard(spriteInfo);
            if (s_clipboardSurface) {
                s_notificationMessage = "Layer pasted!";
                s_notificationTimer = NOTIFICATION_DURATION;
            }
        }
        // Cmd+Z: Undo
        if (ImGui::IsKeyPressed(ImGuiKey_Z) && !io.KeyShift) {
            Undo();
        }
        // Cmd+Shift+Z or Cmd+Y: Redo
        if ((ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyShift) || ImGui::IsKeyPressed(ImGuiKey_Y)) {
            Redo();
        }
    }
    
    // Top bar with filename and save
    ImGui::BeginGroup();
    {
        ImGui::Text("Filename:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(300);
        if (ImGui::InputText("##filename", s_renameBuffer, sizeof(s_renameBuffer))) {
            if (strcmp(s_renameBuffer, spriteInfo->filename.c_str()) != 0) {
                s_spriteModified = true;
            }
        }
        
        ImGui::SameLine();
        
        // Save button
        bool wasModified = s_spriteModified;
        if (wasModified) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.57f, 0.19f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.65f, 0.30f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.90f, 0.50f, 0.15f, 1.00f));
        }
        if (ImGui::Button("Save")) {
            if (SaveSprite(spriteInfo)) {
                s_notificationMessage = "Sprite saved successfully!";
                s_notificationTimer = NOTIFICATION_DURATION;
            } else {
                s_notificationMessage = "Failed to save sprite!";
                s_notificationTimer = NOTIFICATION_DURATION;
            }
        }
        if (wasModified) {
            ImGui::PopStyleColor(3);
        }
        
        ImGui::SameLine();
        ImGui::TextDisabled("Size: %d x %d px", spriteInfo->width, spriteInfo->height);
    }
    ImGui::EndGroup();
    
    ImGui::Separator();
    ImGui::Spacing();
    
    // Layout: Left panel (tools + layers) + Right panel (canvas)
    float leftPanelWidth = 320.0f;
    
    // Left Panel
    ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, 0), true);
    {
        // === HISTORY SECTION ===
        ImGui::Text("History");
        ImGui::Separator();
        
        bool canUndo = !s_undoStack.empty();
        bool canRedo = !s_redoStack.empty();
        bool hasClipboard = (s_clipboardSurface != nullptr);
        
        if (!canUndo) ImGui::BeginDisabled();
        if (ImGui::Button("Undo", ImVec2(60, 0))) {
            Undo();
        }
        if (!canUndo) ImGui::EndDisabled();
        
        ImGui::SameLine();
        if (!canRedo) ImGui::BeginDisabled();
        if (ImGui::Button("Redo", ImVec2(60, 0))) {
            Redo();
        }
        if (!canRedo) ImGui::EndDisabled();
        
        ImGui::SameLine();
        ImGui::TextDisabled("Cmd+Z");
        
        // Copy/Paste layer buttons
        if (ImGui::Button("Copy Layer", ImVec2(95, 0))) {
            CopyLayerToClipboard();
            s_notificationMessage = "Layer copied!";
            s_notificationTimer = NOTIFICATION_DURATION;
        }
        
        ImGui::SameLine();
        if (!hasClipboard) ImGui::BeginDisabled();
        if (ImGui::Button("Paste", ImVec2(95, 0))) {
            PasteFromClipboard(spriteInfo);
            s_notificationMessage = "Layer pasted!";
            s_notificationTimer = NOTIFICATION_DURATION;
        }
        if (!hasClipboard) ImGui::EndDisabled();
        
        ImGui::SameLine();
        ImGui::TextDisabled("Cmd+C/V");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // === DRAWING TOOLS SECTION ===
        ImGui::Text("Drawing Tools");
        ImGui::Separator();
        
        // Tool selection
        ImGui::Text("Tool:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Pencil", s_selectedTool == 0)) {
            s_selectedTool = 0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Eraser", s_selectedTool == 1)) {
            s_selectedTool = 1;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Pipette", s_selectedTool == 2)) {
            s_selectedTool = 2;
        }
        
        ImGui::Spacing();
        
        // Brush size (for pencil and eraser)
        if (s_selectedTool < 2) {
            ImGui::Text("Brush Size:");
            ImGui::SetNextItemWidth(-1);
            ImGui::SliderInt("##brushsize", &s_brushSize, 1, 10);
            ImGui::Spacing();
        }
        
        // Color picker (show current color)
        ImGui::Text("Color:");
        ImGui::ColorEdit4("##color", (float*)&s_currentColor, 
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar);
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // === VIEW CONTROLS SECTION ===
        ImGui::Text("View Controls");
        ImGui::Separator();
        
        // Zoom controls
        ImGui::Text("Zoom:");
        ImGui::SameLine();
        if (ImGui::Button("-##zoom", ImVec2(30, 0))) {
            s_zoomLevel = std::max(s_zoomLevel - 0.5f, 0.5f);
        }
        ImGui::SameLine();
        ImGui::Text("%.0f%%", s_zoomLevel * 100.0f);
        ImGui::SameLine();
        if (ImGui::Button("+##zoom", ImVec2(30, 0))) {
            s_zoomLevel = std::min(s_zoomLevel + 0.5f, 20.0f);
        }
        
        ImGui::Spacing();
        
        // Grid toggle
        ImGui::Checkbox("Show Grid", &s_showGrid);
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // === LAYERS SECTION ===
        ImGui::Text("Layers");
        ImGui::Separator();
        
        if (ImGui::Button("New Layer", ImVec2(-1, 0))) {
            // Create new layer with canvas dimensions
            if (!s_layers.empty()) {
                Layer newLayer;
                newLayer.name = "Layer " + std::to_string(s_layerCounter++);
                newLayer.surface = SDL_CreateRGBSurfaceWithFormat(
                    0, 
                    s_canvasWidth, 
                    s_canvasHeight, 
                    32, 
                    SDL_PIXELFORMAT_RGBA32
                );
                
                // Clear to transparent
                SDL_FillRect(newLayer.surface, nullptr, SDL_MapRGBA(newLayer.surface->format, 0, 0, 0, 0));
                
                newLayer.visible = true;
                newLayer.zOrder = s_layers.size();
                s_layers.push_back(newLayer);
                s_activeLayerIndex = s_layers.size() - 1;
                
                // Clear undo stack for new layer
                FreeUndoStack();
                FreeRedoStack();
                
                // Update composite
                SpriteInfo* info = nullptr;
                for (auto& sprite : s_spriteCache) {
                    if (sprite.filename == g_state.selectedSprite) {
                        info = &sprite;
                        break;
                    }
                }
                if (info) {
                    UpdateEditingSurface(info);
                    UpdateTextureFromSurface(info);
                }
            }
        }
        
        // Import Image button with icon
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.6f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.45f, 0.25f, 1.0f));
        if (ImGui::Button("Import Image...", ImVec2(-1, 0))) {
            // Open file dialog - for now, we'll just show the overlay
            // User can drag & drop or we'll show a path input
            s_showImportOverlay = true;
            s_importOffsetX = 0;
            s_importOffsetY = 0;
            s_importScale = 1.0f;
            s_importOpacity = 1.0f;
            s_importAsNewLayer = true;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Spacing();
        
        // Layer list - use remaining space
        float remainingHeight = ImGui::GetContentRegionAvail().y - 10;
        if (remainingHeight < 100) remainingHeight = 100;
        
        ImGui::BeginChild("LayersList", ImVec2(0, remainingHeight), true);
        {
            for (int i = static_cast<int>(s_layers.size()) - 1; i >= 0; i--) {
                auto& layer = s_layers[i];
                
                ImGui::PushID(i);
                
                // Layer row
                bool isActive = (i == s_activeLayerIndex);
                bool isRenaming = (s_renamingLayerIndex == i);
                
                if (isRenaming) {
                    // Renaming mode - show input text
                    ImGui::SetNextItemWidth(120);
                    if (ImGui::InputText("##rename", s_layerRenameBuffer, sizeof(s_layerRenameBuffer), 
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                        // Enter pressed - apply rename
                        if (strlen(s_layerRenameBuffer) > 0) {
                            layer.name = s_layerRenameBuffer;
                        }
                        s_renamingLayerIndex = -1;
                    }
                    // Cancel on Escape or click elsewhere
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || 
                        (!ImGui::IsItemFocused() && !ImGui::IsItemActive() && ImGui::IsMouseClicked(0))) {
                        s_renamingLayerIndex = -1;
                    }
                    // Set focus on first frame
                    if (ImGui::IsItemVisible() && !ImGui::IsItemActive()) {
                        ImGui::SetKeyboardFocusHere(-1);
                    }
                } else {
                    // Normal mode - show button
                    if (isActive) {
                        ImGui::PushStyleColor(ImGuiCol_Button, MacBlue);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.65f, 1.00f, 1.00f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.50f, 0.90f, 1.00f));
                    }
                    
                    // Layer name button (single click selects, double click renames)
                    if (ImGui::Button(layer.name.c_str(), ImVec2(120, 0))) {
                        s_activeLayerIndex = i;
                    }
                    
                    // Double click to rename
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        s_renamingLayerIndex = i;
                        strncpy(s_layerRenameBuffer, layer.name.c_str(), sizeof(s_layerRenameBuffer) - 1);
                        s_layerRenameBuffer[sizeof(s_layerRenameBuffer) - 1] = '\0';
                    }
                    
                    // Tooltip
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Double-click to rename");
                    }
                    
                    if (isActive) {
                        ImGui::PopStyleColor(3);
                    }
                }
                
                ImGui::SameLine();
                
                // Visibility checkbox
                bool visible = layer.visible;
                if (ImGui::Checkbox("##vis", &visible)) {
                    layer.visible = visible;
                    
                    // Update composite
                    SpriteInfo* info = nullptr;
                    for (auto& sprite : s_spriteCache) {
                        if (sprite.filename == g_state.selectedSprite) {
                            info = &sprite;
                            break;
                        }
                    }
                    if (info) {
                        UpdateEditingSurface(info);
                        UpdateTextureFromSurface(info);
                    }
                }
                
                ImGui::SameLine();
                
                // Move up button
                if (i < static_cast<int>(s_layers.size()) - 1) {
                    if (ImGui::SmallButton("▲")) {
                        std::swap(s_layers[i], s_layers[i + 1]);
                        if (s_activeLayerIndex == i) s_activeLayerIndex = i + 1;
                        else if (s_activeLayerIndex == i + 1) s_activeLayerIndex = i;
                        
                        s_layers[i].zOrder = i;
                        s_layers[i + 1].zOrder = i + 1;
                        
                        SpriteInfo* info = nullptr;
                        for (auto& sprite : s_spriteCache) {
                            if (sprite.filename == g_state.selectedSprite) {
                                info = &sprite;
                                break;
                            }
                        }
                        if (info) {
                            UpdateEditingSurface(info);
                            UpdateTextureFromSurface(info);
                        }
                    }
                    ImGui::SameLine();
                }
                
                // Move down button
                if (i > 0) {
                    if (ImGui::SmallButton("▼")) {
                        std::swap(s_layers[i], s_layers[i - 1]);
                        if (s_activeLayerIndex == i) s_activeLayerIndex = i - 1;
                        else if (s_activeLayerIndex == i - 1) s_activeLayerIndex = i;
                        
                        s_layers[i].zOrder = i;
                        s_layers[i - 1].zOrder = i - 1;
                        
                        SpriteInfo* info = nullptr;
                        for (auto& sprite : s_spriteCache) {
                            if (sprite.filename == g_state.selectedSprite) {
                                info = &sprite;
                                break;
                            }
                        }
                        if (info) {
                            UpdateEditingSurface(info);
                            UpdateTextureFromSurface(info);
                        }
                    }
                    ImGui::SameLine();
                }
                
                // Delete button (only if more than 1 layer)
                if (s_layers.size() > 1) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                    if (ImGui::SmallButton("X")) {
                        // Free the layer surface
                        if (s_layers[i].surface) {
                            SDL_FreeSurface(s_layers[i].surface);
                        }
                        s_layers.erase(s_layers.begin() + i);
                        
                        // Adjust active layer index
                        if (s_activeLayerIndex >= static_cast<int>(s_layers.size())) {
                            s_activeLayerIndex = s_layers.size() - 1;
                        }
                        
                        // Update z-order for remaining layers
                        for (int j = 0; j < static_cast<int>(s_layers.size()); j++) {
                            s_layers[j].zOrder = j;
                        }
                        
                        // Clear undo/redo stacks
                        FreeUndoStack();
                        FreeRedoStack();
                        
                        // Update composite
                        SpriteInfo* info = nullptr;
                        for (auto& sprite : s_spriteCache) {
                            if (sprite.filename == g_state.selectedSprite) {
                                info = &sprite;
                                break;
                            }
                        }
                        if (info) {
                            UpdateEditingSurface(info);
                            UpdateTextureFromSurface(info);
                        }
                    }
                    ImGui::PopStyleColor(3);
                }
                
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndChild(); // End Left Panel
    
    ImGui::SameLine();
    
    // Right Panel - Canvas area with dark background
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.09f, 1.00f));
    ImGui::BeginChild("Canvas", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        // Get available space
        ImVec2 availSize = ImGui::GetContentRegionAvail();
        float regionWidth = availSize.x - 20; // Account for scrollbar
        float regionHeight = availSize.y - 20;
        
        // Calculate scale to fit both width and height
        float scaleW = regionWidth / spriteInfo->width;
        float scaleH = regionHeight / spriteInfo->height;
        float baseScale = std::min(scaleW, scaleH);
        float displayScale = baseScale * s_zoomLevel;
        
        float displayW = spriteInfo->width * displayScale;
        float displayH = spriteInfo->height * displayScale;
        
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();
        
        // Draw sprite
        ImGui::Image(spriteInfo->textureId, ImVec2(displayW, displayH));
        
        // Check if mouse is over the image
        bool isHovered = ImGui::IsItemHovered();
        
        // Draw grid overlay if enabled
        if (s_showGrid && displayScale >= 2.0f) {
            ImU32 gridColor = IM_COL32(80, 80, 80, 120);
            
            // Vertical lines
            for (int x = 0; x <= spriteInfo->width; x++) {
                float posX = canvasPos.x + x * displayScale;
                drawList->AddLine(
                    ImVec2(posX, canvasPos.y),
                    ImVec2(posX, canvasPos.y + displayH),
                    gridColor
                );
            }
            
            // Horizontal lines
            for (int y = 0; y <= spriteInfo->height; y++) {
                float posY = canvasPos.y + y * displayScale;
                drawList->AddLine(
                    ImVec2(canvasPos.x, posY),
                    ImVec2(canvasPos.x + displayW, posY),
                    gridColor
                );
            }
        }
        
        // Handle drawing and pipette
        if (isHovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            
            ImVec2 mousePos = io.MousePos;
            int pixelX = (int)((mousePos.x - canvasPos.x) / displayScale);
            int pixelY = (int)((mousePos.y - canvasPos.y) / displayScale);
            
            if (pixelX >= 0 && pixelX < spriteInfo->width && 
                pixelY >= 0 && pixelY < spriteInfo->height) {
                
                // Draw brush preview (for pencil and eraser only)
                if (s_selectedTool < 2) {
                    for (int bx = -s_brushSize/2; bx <= s_brushSize/2; bx++) {
                        for (int by = -s_brushSize/2; by <= s_brushSize/2; by++) {
                            int px = pixelX + bx;
                            int py = pixelY + by;
                            
                            if (px >= 0 && px < spriteInfo->width && 
                                py >= 0 && py < spriteInfo->height) {
                                
                                float rectX = canvasPos.x + px * displayScale;
                                float rectY = canvasPos.y + py * displayScale;
                                
                                // Semi-transparent preview
                                ImU32 previewColor;
                                if (s_selectedTool == 0) {
                                    // Pencil: show color with 50% opacity
                                    ImVec4 previewCol = s_currentColor;
                                    previewCol.w = 0.5f;
                                    previewColor = ImGui::ColorConvertFloat4ToU32(previewCol);
                                } else {
                                    // Eraser: gray semi-transparent
                                    previewColor = IM_COL32(100, 100, 100, 128);
                                }
                                
                                drawList->AddRectFilled(
                                    ImVec2(rectX, rectY),
                                    ImVec2(rectX + displayScale, rectY + displayScale),
                                    previewColor
                                );
                            }
                        }
                    }
                } else {
                    // Pipette: show crosshair preview
                    float rectX = canvasPos.x + pixelX * displayScale;
                    float rectY = canvasPos.y + pixelY * displayScale;
                    drawList->AddRect(
                        ImVec2(rectX, rectY),
                        ImVec2(rectX + displayScale, rectY + displayScale),
                        IM_COL32(255, 255, 255, 255),
                        0.0f, 0, 2.0f
                    );
                }
            }
            
            if (ImGui::IsMouseDown(0)) {
                if (s_selectedTool == 2) {
                    // Pipette - pick color from composite surface
                    if (pixelX >= 0 && pixelX < spriteInfo->width && 
                        pixelY >= 0 && pixelY < spriteInfo->height) {
                        
                        // Get color from the active layer
                        if (s_activeLayerIndex < static_cast<int>(s_layers.size()) && s_layers[s_activeLayerIndex].surface) {
                            SDL_Surface* activeSurface = s_layers[s_activeLayerIndex].surface;
                            
                            if (SDL_MUSTLOCK(activeSurface)) {
                                SDL_LockSurface(activeSurface);
                            }
                            
                            Uint32* pixels = (Uint32*)activeSurface->pixels;
                            int pitch = activeSurface->pitch / 4;
                            Uint32 pixel = pixels[pixelY * pitch + pixelX];
                            
                            Uint8 r, g, b, a;
                            SDL_GetRGBA(pixel, activeSurface->format, &r, &g, &b, &a);
                            
                            s_currentColor.x = r / 255.0f;
                            s_currentColor.y = g / 255.0f;
                            s_currentColor.z = b / 255.0f;
                            s_currentColor.w = a / 255.0f;
                            
                            if (SDL_MUSTLOCK(activeSurface)) {
                                SDL_UnlockSurface(activeSurface);
                            }
                            
                            // Switch to pencil after picking
                            s_selectedTool = 0;
                        }
                    }
                } else {
                    // Pencil or Eraser - draw
                    if (!s_isDrawing) {
                        PushUndoState();
                        s_isDrawing = true;
                    }
                    
                    if (pixelX >= 0 && pixelX < spriteInfo->width && 
                        pixelY >= 0 && pixelY < spriteInfo->height) {
                        
                        // Get active layer
                        if (s_activeLayerIndex < static_cast<int>(s_layers.size()) && s_layers[s_activeLayerIndex].surface) {
                            SDL_Surface* activeSurface = s_layers[s_activeLayerIndex].surface;
                            
                            // Apply brush strokes
                            for (int bx = -s_brushSize/2; bx <= s_brushSize/2; bx++) {
                                for (int by = -s_brushSize/2; by <= s_brushSize/2; by++) {
                                    int px = pixelX + bx;
                                    int py = pixelY + by;
                                    
                                    if (px >= 0 && px < spriteInfo->width && 
                                        py >= 0 && py < spriteInfo->height) {
                                        
                                        // Actually modify the active layer surface
                                        if (SDL_MUSTLOCK(activeSurface)) {
                                            SDL_LockSurface(activeSurface);
                                        }
                                        
                                        Uint32* pixels = (Uint32*)activeSurface->pixels;
                                        int pitch = activeSurface->pitch / 4;
                                        
                                        if (s_selectedTool == 0) {
                                            // Pencil - draw color
                                            Uint8 r = (Uint8)(s_currentColor.x * 255);
                                            Uint8 g = (Uint8)(s_currentColor.y * 255);
                                            Uint8 b = (Uint8)(s_currentColor.z * 255);
                                            Uint8 a = (Uint8)(s_currentColor.w * 255);
                                            pixels[py * pitch + px] = SDL_MapRGBA(activeSurface->format, r, g, b, a);
                                        } else {
                                            // Eraser - transparent
                                            pixels[py * pitch + px] = SDL_MapRGBA(activeSurface->format, 0, 0, 0, 0);
                                        }
                                        
                                        if (SDL_MUSTLOCK(activeSurface)) {
                                            SDL_UnlockSurface(activeSurface);
                                        }
                                        
                                        s_spriteModified = true;
                                    }
                                }
                            }
                            
                            // Update composite texture
                            UpdateEditingSurface(spriteInfo);
                            UpdateTextureFromSurface(spriteInfo);
                        }
                    }
                }
            } else {
                // Released mouse - end drawing
                s_isDrawing = false;
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void RenderSpriteEditor() {
    if (g_state.selectedSprite.empty()) {
        ImGui::TextDisabled("No sprite selected. Choose a sprite from the left panel.");
        return;
    }
    
    // Handle keyboard shortcuts
    ImGuiIO& io = ImGui::GetIO();
    bool modPressed = io.KeySuper || io.KeyCtrl; // Cmd on macOS, Ctrl on Windows/Linux
    
    if (modPressed && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (io.KeyShift) {
            // Cmd+Shift+Z / Ctrl+Shift+Z = Redo
            Redo();
        } else {
            // Cmd+Z / Ctrl+Z = Undo
            Undo();
        }
    }
    
    // Find selected sprite info
    SpriteInfo* selectedInfo = nullptr;
    for (auto& info : s_spriteCache) {
        if (info.filename == g_state.selectedSprite) {
            selectedInfo = &info;
            break;
        }
    }
    
    if (!selectedInfo) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: Sprite not found");
        return;
    }
    
    // Header with mode toggle button
    ImGui::BeginGroup();
    {
        ImGui::Text("Sprite: %s", selectedInfo->filename.c_str());
        ImGui::SameLine();
        
        // Push button to the right
        float buttonWidth = 150.0f;
        float availWidth = ImGui::GetContentRegionAvail().x;
        if (availWidth > buttonWidth) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - buttonWidth);
        }
        
        // Toggle button with macOS colors
        if (g_state.pixelEditorMode) {
            // Show Preview button
            ImGui::PushStyleColor(ImGuiCol_Button, MacBlue);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.65f, 1.00f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.50f, 0.90f, 1.00f));
            if (ImGui::Button("Show Preview", ImVec2(buttonWidth, 0))) {
                g_state.pixelEditorMode = false;
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, MacGreen);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.88f, 0.42f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.75f, 0.30f, 1.00f));
            if (ImGui::Button("Pixel Editor", ImVec2(buttonWidth, 0))) {
                g_state.pixelEditorMode = true;
            }
            ImGui::PopStyleColor(3);
        }
    }
    ImGui::EndGroup();
    
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render appropriate mode
    if (g_state.pixelEditorMode) {
        RenderPixelEditor(selectedInfo);
    } else {
        // Preview mode
        ImGui::BeginGroup();
        {
            ImGui::Text("File Information");
            ImGui::Spacing();
            
            ImGui::Text("Name:");
            ImGui::SameLine(120);
            ImGui::TextColored(MacBlue, "%s", selectedInfo->filename.c_str());
            
            ImGui::Text("Type:");
            ImGui::SameLine(120);
            ImGui::TextColored(MacGreen, "%s", selectedInfo->extension.c_str());
            
            ImGui::Text("Size:");
            ImGui::SameLine(120);
            ImGui::Text("%s", FormatFileSize(selectedInfo->fileSize).c_str());
            
            if (selectedInfo->width > 0 && selectedInfo->height > 0) {
                ImGui::Text("Dimensions:");
                ImGui::SameLine(120);
                ImGui::Text("%d x %d px", selectedInfo->width, selectedInfo->height);
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Sprite Preview
            ImGui::Separator();
            ImGui::Text("Preview");
            ImGui::Spacing();
            
            if (selectedInfo->textureId) {
                // Calculate preview size to fit width
                float regionWidth = ImGui::GetContentRegionAvail().x;
                float scale = 1.0f;
                
                // Scale to fit width, maintaining aspect ratio
                if (selectedInfo->width > regionWidth) {
                    scale = regionWidth / selectedInfo->width;
                } else {
                    // If smaller than region, scale up to use most of the width
                    scale = std::min(regionWidth / selectedInfo->width, 4.0f); // Max 4x upscale
                }
                
                float displayW = selectedInfo->width * scale;
                float displayH = selectedInfo->height * scale;
                
                // Center the image
                float offsetX = (regionWidth - displayW) * 0.5f;
                if (offsetX > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
                
                ImGui::Image(selectedInfo->textureId, ImVec2(displayW, displayH));
                
                // Zoom info
                if (scale != 1.0f) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("(%.0f%% scale)", scale * 100.0f);
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Failed to load preview");
            }
        }
        ImGui::EndGroup();
    }
}

void RenderSpriteEditorUI() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_MenuBar;
    
    ImGui::Begin("Sprite Editor", nullptr, windowFlags);
    
    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        // Back button with blue accent
        ImGui::PushStyleColor(ImGuiCol_Button, MacBlue);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.65f, 1.00f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.50f, 0.90f, 1.00f));
        if (ImGui::Button("<< Menu")) {
            g_state.mode = EditorMode::MainMenu;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        
        // Current sprite indicator
        if (!g_state.selectedSprite.empty()) {
            ImGui::Text("%s", g_state.selectedSprite.c_str());
        } else {
            ImGui::TextDisabled("No sprite selected");
        }
        
        ImGui::EndMenuBar();
    }
    
    // Main Layout: Left Panel (Sprites) + Right Panel (Editor)
    float panelWidth = ImGui::GetContentRegionAvail().x * 0.20f;
    
    // Left Panel: Sprite List
    ImGui::BeginChild("SpriteList", ImVec2(panelWidth, 0), true);
    {
        if (ImGui::CollapsingHeader("Sprites", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            
            if (ImGui::Button("Refresh", ImVec2(-1, 0))) {
                RefreshSpriteList();
            }
            
            ImGui::Spacing();
            
            // Display sprite count
            ImGui::TextDisabled("(%zu files)", g_state.spriteFiles.size());
            ImGui::Spacing();
            
            // Drag and drop hint
            ImGui::TextDisabled("Drag & drop to import");
            ImGui::TextDisabled("(.png, .gif, .jpg, .bmp)");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Sprite list
            for (const auto& sprite : g_state.spriteFiles) {
                bool isSelected = (sprite == g_state.selectedSprite);
            
            if (ImGui::Selectable(sprite.c_str(), isSelected)) {
                if (sprite != g_state.selectedSprite) {
                    SelectSprite(sprite);
                }
            }
        }
        
        ImGui::Unindent();
        }
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Right Panel: Sprite Editor
    ImGui::BeginChild("Editor", ImVec2(0, 0), true);
    {
        RenderSpriteEditor();
    }
    ImGui::EndChild();
    
    ImGui::End();
    
    // Import Image Overlay Modal
    if (s_showImportOverlay) {
        ImGui::OpenPopup("Import Image##Modal");
    }
    
    // Center the modal and set a taller initial height (only on appearing)
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Appearing);
    
    if (ImGui::BeginPopupModal("Import Image##Modal", &s_showImportOverlay, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(MacBlue, "Import an image to overlay on your sprite");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Instructions
        ImGui::TextWrapped("Drag & drop an image file here, or browse sprites below");
        ImGui::Spacing();
        
        // Browse sprites button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.6f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.45f, 0.7f, 1.0f));
        if (ImGui::Button("Browse Sprites...", ImVec2(-1, 0))) {
            ImGui::OpenPopup("SelectSpritePopup");
        }
        ImGui::PopStyleColor(3);
        
        // Sprite selection popup
        if (ImGui::BeginPopup("SelectSpritePopup")) {
            ImGui::Text("Select a sprite to import:");
            ImGui::Separator();
            
            // List sprites from cache
            for (const auto& sprite : s_spriteCache) {
                if (sprite.filename.empty()) continue;
                
                // Show sprite filename
                if (ImGui::Selectable(sprite.filename.c_str())) {
                    // Load this sprite as import preview
                    std::string spritePath = g_state.spritePath + "/" + sprite.filename;
                    if (LoadImportPreview(spritePath)) {
                        s_notificationMessage = "Sprite loaded for import!";
                        s_notificationTimer = NOTIFICATION_DURATION;
                    } else {
                        s_notificationMessage = "Failed to load sprite!";
                        s_notificationTimer = NOTIFICATION_DURATION;
                    }
                    ImGui::CloseCurrentPopup();
                }
                
                // Tooltip with size info
                if (ImGui::IsItemHovered() && sprite.textureId) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%d x %d px", sprite.width, sprite.height);
                    ImGui::EndTooltip();
                }
            }
            
            ImGui::EndPopup();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Preview and options (only if image loaded)
        if (s_importPreviewSurface) {
            ImGui::Separator();
            ImGui::Spacing();
            
            // Show preview info
            ImGui::Text("Image: %s", fs::path(s_importFilePath).filename().string().c_str());
            ImGui::Text("Size: %d x %d px", s_importPreviewSurface->w, s_importPreviewSurface->h);
            
            // Target info
            if (!s_layers.empty() && s_layers[0].surface) {
                ImGui::Text("Target Canvas: %d x %d px", s_layers[0].surface->w, s_layers[0].surface->h);
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Position controls
            ImGui::Text("Position:");
            ImGui::SetNextItemWidth(150);
            ImGui::InputInt("X Offset##import", &s_importOffsetX);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::InputInt("Y Offset##import", &s_importOffsetY);
            
            // Center button
            ImGui::SameLine();
            if (ImGui::Button("Center")) {
                if (!s_layers.empty() && s_layers[0].surface) {
                    int scaledW = static_cast<int>(s_importPreviewSurface->w * s_importScale);
                    int scaledH = static_cast<int>(s_importPreviewSurface->h * s_importScale);
                    s_importOffsetX = (s_layers[0].surface->w - scaledW) / 2;
                    s_importOffsetY = (s_layers[0].surface->h - scaledH) / 2;
                }
            }
            
            ImGui::Spacing();
            
            // Scale control
            ImGui::Text("Scale:");
            ImGui::SetNextItemWidth(200);
            ImGui::SliderFloat("##importscale", &s_importScale, 0.1f, 5.0f, "%.2fx");
            ImGui::SameLine();
            if (ImGui::Button("Reset##scale")) {
                s_importScale = 1.0f;
            }
            
            // Fit buttons
            ImGui::SameLine();
            if (!s_layers.empty() && s_layers[0].surface && ImGui::Button("Fit")) {
                float scaleW = static_cast<float>(s_layers[0].surface->w) / s_importPreviewSurface->w;
                float scaleH = static_cast<float>(s_layers[0].surface->h) / s_importPreviewSurface->h;
                s_importScale = std::min(scaleW, scaleH);
                // Auto-center after fit
                int scaledW = static_cast<int>(s_importPreviewSurface->w * s_importScale);
                int scaledH = static_cast<int>(s_importPreviewSurface->h * s_importScale);
                s_importOffsetX = (s_layers[0].surface->w - scaledW) / 2;
                s_importOffsetY = (s_layers[0].surface->h - scaledH) / 2;
            }
            
            ImGui::Spacing();
            
            // Opacity control
            ImGui::Text("Opacity:");
            ImGui::SetNextItemWidth(200);
            ImGui::SliderFloat("##importopacity", &s_importOpacity, 0.0f, 1.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Import mode
            ImGui::Text("Import Mode:");
            if (ImGui::RadioButton("New Layer", s_importAsNewLayer)) {
                s_importAsNewLayer = true;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Merge to Active Layer", !s_importAsNewLayer)) {
                s_importAsNewLayer = false;
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Action buttons
            ImGui::PushStyleColor(ImGuiCol_Button, MacGreen);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.88f, 0.42f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.75f, 0.30f, 1.0f));
            
            if (ImGui::Button("Apply Import", ImVec2(150, 30))) {
                SpriteInfo* info = nullptr;
                for (auto& sprite : s_spriteCache) {
                    if (sprite.filename == g_state.selectedSprite) {
                        info = &sprite;
                        break;
                    }
                }
                ApplyImportedImage(info);
                s_notificationMessage = "Image imported successfully!";
                s_notificationTimer = NOTIFICATION_DURATION;
            }
            ImGui::PopStyleColor(3);
            
            ImGui::SameLine();
        } else {
            // No image loaded yet - show helpful message
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "No image loaded");
            ImGui::Spacing();
            ImGui::TextWrapped("Please drag & drop an image file onto this window, or select one from the sprite list on the left.");
        }
        
        // Cancel button (always visible)
        if (ImGui::Button("Cancel", ImVec2(100, 30))) {
            FreeImportPreview();
            s_showImportOverlay = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Render notification overlay
    if (s_notificationTimer > 0.0f) {
        s_notificationTimer -= ImGui::GetIO().DeltaTime;
        
        // Calculate fade
        float alpha = std::min(s_notificationTimer / 0.5f, 1.0f);
        
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 windowPos = ImVec2(viewport->Pos.x + viewport->Size.x - 320, viewport->Pos.y + 50);
        
        ImGui::SetNextWindowPos(windowPos);
        ImGui::SetNextWindowBgAlpha(0.85f * alpha);
        
        ImGuiWindowFlags notifFlags = ImGuiWindowFlags_NoTitleBar | 
                                       ImGuiWindowFlags_NoResize | 
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoFocusOnAppearing |
                                       ImGuiWindowFlags_NoNav;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 10));
        
        // Green for success, red for error
        bool isSuccess = s_notificationMessage.find("success") != std::string::npos;
        ImVec4 borderColor = isSuccess ? MacGreen : ImVec4(1.0f, 0.3f, 0.3f, alpha);
        ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        
        if (ImGui::Begin("##Notification", nullptr, notifFlags)) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
            ImGui::Text("%s", s_notificationMessage.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::End();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }
}

bool IsImportOverlayActive() {
    return s_showImportOverlay;
}

bool HandleImportDroppedFile(const std::string& filePath) {
    if (!s_showImportOverlay) return false;
    
    if (!IsSupportedSpriteExtension(fs::path(filePath).extension().string())) {
        return false;
    }
    
    return LoadImportPreview(filePath);
}

} // namespace AssetEditor
