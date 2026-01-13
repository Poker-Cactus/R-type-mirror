/**
 * @file EditorState.cpp
 * @brief Editor state management implementation
 */

#include "EditorState.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace AssetEditor {

// Global state instance
EditorState g_state;

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
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }
}

bool LoadFile(const std::string& filename) {
    std::string fullPath = g_state.configPath + "/" + filename;
    try {
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open: " << filename << std::endl;
            return false;
        }
        g_state.currentJson = json::parse(file);
        g_state.selectedFile = filename;
        g_state.modified = false;
        std::cout << "Loaded: " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return false;
    }
}

bool SaveFile() {
    if (g_state.selectedFile.empty()) return false;
    
    std::string fullPath = g_state.configPath + "/" + g_state.selectedFile;
    try {
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "Failed to save: " << g_state.selectedFile << std::endl;
            return false;
        }
        file << g_state.currentJson.dump(2);
        g_state.modified = false;
        std::cout << "Saved: " << g_state.selectedFile << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Save error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace AssetEditor
