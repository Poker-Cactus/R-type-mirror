/**
 * @file SpriteEditor.h
 * @brief Sprite editor interface for managing game sprites
 * 
 * Provides functionality to browse, view, and edit sprite metadata
 * from the client/assets/sprites directory.
 */

#pragma once

#include <string>
#include <vector>

struct SDL_Renderer;  // Forward declaration

namespace AssetEditor {

/**
 * @brief Information about a sprite file
 */
struct SpriteInfo {
    std::string filename;      ///< Name of the sprite file
    std::string fullPath;      ///< Full path to the sprite file
    std::string extension;     ///< File extension (.png, .gif, etc.)
    size_t fileSize = 0;       ///< File size in bytes
    int width = 0;             ///< Image width (loaded from file)
    int height = 0;            ///< Image height (loaded from file)
    void* textureId = nullptr; ///< ImGui texture ID for preview
};

/**
 * @brief Refresh the list of sprite files from assets directory
 */
void RefreshSpriteList();

/**
 * @brief Set the SDL renderer for texture loading
 * @param renderer SDL renderer from main application
 */
void SetSpriteRenderer(SDL_Renderer* renderer);

/**
 * @brief Clean up all loaded sprite textures
 */
void CleanupSpriteTextures();

/**
 * @brief Select a sprite for viewing/editing
 * @param filename Name of the sprite file to select
 */
void SelectSprite(const std::string& filename);

/**
 * @brief Main sprite editor UI rendering function
 * 
 * Renders the complete sprite editor interface with:
 * - File browser panel (left)
 * - Sprite viewer/editor panel (right)
 * - Menu bar with navigation
 */
void RenderSpriteEditorUI();

/**
 * @brief Render the sprite preview/editor panel
 */
void RenderSpriteEditor();

/**
 * @brief Import a sprite file via drag and drop
 * @param filePath Full path to the file to import
 * @return true if import was successful
 */
bool ImportSpriteFile(const std::string& filePath);

/**
 * @brief Check if a file extension is supported for sprites
 * @param extension File extension (e.g., ".png")
 * @return true if the extension is supported
 */
bool IsSupportedSpriteExtension(const std::string& extension);

/**
 * @brief Check if the import image overlay is currently shown
 * @return true if import overlay is active
 */
bool IsImportOverlayActive();

/**
 * @brief Handle a dropped file for import overlay
 * @param filePath Full path to the dropped file
 * @return true if file was loaded for import
 */
bool HandleImportDroppedFile(const std::string& filePath);

} // namespace AssetEditor
