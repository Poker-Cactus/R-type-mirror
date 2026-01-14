/**
 * @file EditorState.hpp
 * @brief Editor state management and data structures
 *
 * Modern C++ implementation with RAII patterns.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>

namespace AssetEditor {

using Json = nlohmann::json;

/**
 * @brief Editor operating modes
 */
enum class EditorMode {
    MainMenu,      ///< Main menu selection screen
    JsonEditor,    ///< JSON file editor mode
    SpriteEditor,  ///< Sprite editor mode
};

/**
 * @brief Global editor state container
 *
 * Manages all shared state across the application including file lists,
 * current selections, and configuration paths.
 */
struct EditorState {
    EditorMode mode = EditorMode::MainMenu;

    // JSON Editor state
    std::vector<std::string> jsonFiles;
    std::string selectedFile;
    Json currentJson;
    bool modified = false;

    // Sprite Editor state
    std::vector<std::string> spriteFiles;
    std::string selectedSprite;
    std::string spritePath = SPRITE_PATH;
    bool pixelEditorMode = false;

    // Config path
    std::string configPath = CONFIG_PATH;
};

/// Global state instance
extern EditorState g_state;

/**
 * @brief Refresh the list of JSON files from config directory
 */
void RefreshFileList();

/**
 * @brief Load a JSON file into the editor
 * @param filename Name of the file to load (without path)
 * @return true if loaded successfully
 */
[[nodiscard]] bool LoadFile(const std::string& filename);

/**
 * @brief Save the current JSON file
 * @return true if saved successfully
 */
[[nodiscard]] bool SaveFile();

} // namespace AssetEditor
