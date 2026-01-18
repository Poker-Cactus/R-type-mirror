/**
 * @file EditorState.cpp
 * @brief Editor state management implementation
 */

#include "EditorState.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace AssetEditor
{

// Global state instance
EditorState g_state;

void RefreshFileList()
{
  g_state.jsonFiles.clear();
  try {
    for (const auto &entry : fs::directory_iterator(g_state.configPath)) {
      if (entry.path().extension() == ".json") {
        g_state.jsonFiles.push_back(entry.path().filename().string());
      }
    }
    std::sort(g_state.jsonFiles.begin(), g_state.jsonFiles.end());
  } catch (const std::exception &e) {
    std::cerr << "[AssetEditor] Error scanning directory: " << e.what() << std::endl;
  }
}

bool LoadFile(const std::string &filename)
{
  const std::string fullPath = g_state.configPath + "/" + filename;
  try {
    std::ifstream file(fullPath);
    if (!file.is_open()) {
      std::cerr << "[AssetEditor] Failed to open: " << filename << std::endl;
      return false;
    }
    g_state.currentJson = Json::parse(file);
    g_state.selectedFile = filename;
    g_state.modified = false;
    std::cout << "[AssetEditor] Loaded: " << filename << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[AssetEditor] Parse error: " << e.what() << std::endl;
    return false;
  }
}

bool SaveFile()
{
  if (g_state.selectedFile.empty()) {
    return false;
  }

  const std::string fullPath = g_state.configPath + "/" + g_state.selectedFile;
  try {
    std::ofstream file(fullPath);
    if (!file.is_open()) {
      std::cerr << "[AssetEditor] Failed to save: " << g_state.selectedFile << std::endl;
      return false;
    }
    file << g_state.currentJson.dump(2);
    g_state.modified = false;
    std::cout << "[AssetEditor] Saved: " << g_state.selectedFile << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[AssetEditor] Save error: " << e.what() << std::endl;
    return false;
  }
}

} // namespace AssetEditor
