/**
 * @file EditorState.h
 * @brief Editor state management and data structures
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace AssetEditor {

/**
 * @brief Editor operating modes
 */
enum class EditorMode {
    MainMenu,      ///< Main menu selection screen
    JsonEditor,    ///< JSON file editor mode
    SpriteEditor,  ///< Sprite editor mode (future)
};

/**
 * @brief Global editor state
 */
struct EditorState {
    EditorMode mode = EditorMode::MainMenu;
    
    // JSON Editor state
    std::vector<std::string> jsonFiles;
    std::string selectedFile;
    json currentJson;
    bool modified = false;
    
    // Sprite Editor state
    std::vector<std::string> spriteFiles;
    std::string selectedSprite;
    std::string spritePath = SPRITE_PATH;
    bool pixelEditorMode = false;  // Toggle between preview and pixel editor
    
    // Config path
    std::string configPath = CONFIG_PATH;
};

// Global state instance
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
bool LoadFile(const std::string& filename);

/**
 * @brief Save the current JSON file
 * @return true if saved successfully
 */
bool SaveFile();

} // namespace AssetEditor
