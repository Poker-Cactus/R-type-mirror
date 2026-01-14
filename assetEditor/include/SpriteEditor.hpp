/**
 * @file SpriteEditor.hpp
 * @brief Sprite editor interface for managing game sprites
 *
 * Provides functionality to browse, view, and edit sprite metadata
 * from the client/assets/sprites directory.
 *
 * Uses RAII patterns with custom SDL deleters for safe resource management.
 */

#pragma once

#include <SDL.h>
#include <memory>
#include <string>
#include <vector>

namespace AssetEditor
{

// ═══════════════════════════════════════════════════════════════════════════
// RAII Wrappers for SDL Resources
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Custom deleter for SDL_Surface
 */
struct SurfaceDeleter {
  void operator()(SDL_Surface *surface) const noexcept
  {
    if (surface) {
      SDL_FreeSurface(surface);
    }
  }
};

/**
 * @brief Custom deleter for SDL_Texture
 */
struct TextureDeleter {
  void operator()(SDL_Texture *texture) const noexcept
  {
    if (texture) {
      SDL_DestroyTexture(texture);
    }
  }
};

/// Smart pointer type for SDL_Surface with automatic cleanup
using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

/// Smart pointer type for SDL_Texture with automatic cleanup (shared for copyability in SpriteInfo)
using TexturePtr = std::shared_ptr<SDL_Texture>;

/**
 * @brief Create a SurfacePtr from raw SDL_Surface
 * @param surface Raw SDL_Surface pointer (takes ownership)
 * @return Managed SurfacePtr
 */
[[nodiscard]] inline SurfacePtr MakeSurfacePtr(SDL_Surface *surface)
{
  return SurfacePtr(surface);
}

/**
 * @brief Create a TexturePtr from raw SDL_Texture
 * @param texture Raw SDL_Texture pointer (takes ownership)
 * @return Managed TexturePtr (shared)
 */
[[nodiscard]] inline TexturePtr MakeTexturePtr(SDL_Texture *texture)
{
  return TexturePtr(texture, TextureDeleter{});
}

/**
 * @brief Clone an SDL_Surface safely
 * @param source Surface to clone
 * @return New managed surface or nullptr on failure
 */
[[nodiscard]] SurfacePtr CloneSurface(SDL_Surface *source);

// ═══════════════════════════════════════════════════════════════════════════
// Sprite Information
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Information about a sprite file
 *
 * Stores metadata and texture reference for sprite preview/editing.
 */
struct SpriteInfo {
  std::string filename; ///< Name of the sprite file
  std::string fullPath; ///< Full path to the sprite file
  std::string extension; ///< File extension (.png, .gif, etc.)
  size_t fileSize = 0; ///< File size in bytes
  int width = 0; ///< Image width in pixels
  int height = 0; ///< Image height in pixels
  TexturePtr texture; ///< ImGui texture for preview (managed)

  /// Get raw texture pointer for ImGui
  [[nodiscard]] void *GetTextureID() const noexcept { return texture.get(); }
};

// ═══════════════════════════════════════════════════════════════════════════
// Layer System
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Represents a single layer in the sprite editor
 *
 * Each layer has its own surface for drawing operations.
 */
struct Layer {
  std::string name; ///< Layer display name
  SurfacePtr surface; ///< Layer pixel data (managed)
  bool visible = true; ///< Layer visibility toggle
  int zOrder = 0; ///< Layer stacking order

  /// Default constructor
  Layer() = default;

  /// Move constructor
  Layer(Layer &&) = default;

  /// Move assignment
  Layer &operator=(Layer &&) = default;

  /// Non-copyable (surfaces are unique)
  Layer(const Layer &) = delete;
  Layer &operator=(const Layer &) = delete;
};

// ═══════════════════════════════════════════════════════════════════════════
// Public API
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Refresh the list of sprite files from assets directory
 */
void RefreshSpriteList();

/**
 * @brief Set the SDL renderer for texture loading
 * @param renderer SDL renderer from main application
 */
void SetSpriteRenderer(SDL_Renderer *renderer);

/**
 * @brief Clean up all loaded sprite textures
 */
void CleanupSpriteTextures();

/**
 * @brief Select a sprite for viewing/editing
 * @param filename Name of the sprite file to select
 */
void SelectSprite(const std::string &filename);

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
[[nodiscard]] bool ImportSpriteFile(const std::string &filePath);

/**
 * @brief Check if a file extension is supported for sprites
 * @param extension File extension (e.g., ".png")
 * @return true if the extension is supported
 */
[[nodiscard]] bool IsSupportedSpriteExtension(const std::string &extension);

/**
 * @brief Check if the import image overlay is currently shown
 * @return true if import overlay is active
 */
[[nodiscard]] bool IsImportOverlayActive();

/**
 * @brief Handle a dropped file for import overlay
 * @param filePath Full path to the dropped file
 * @return true if file was loaded for import
 */
[[nodiscard]] bool HandleImportDroppedFile(const std::string &filePath);

} // namespace AssetEditor
