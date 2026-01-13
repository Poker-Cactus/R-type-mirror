/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Highscore.cpp - Highscore management implementation
*/

#include "../include/Highscore.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

HighscoreManager::HighscoreManager()
{
  // Try to load existing highscores on construction
  loadFromFile();
}

bool HighscoreManager::loadFromFile()
{
  try {
    const fs::path filePath = getHighscoreFilePath();

    if (!fs::exists(filePath)) {
      // File doesn't exist, start with empty highscores
      m_highscores.clear();
      return true;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "[HighscoreManager] Failed to open highscore file for reading: " << filePath << std::endl;
      return false;
    }

    nlohmann::json json;
    file >> json;

    m_highscores = json.get<std::vector<HighscoreEntry>>();
    sortHighscores();  // Ensure they're sorted

    return true;
  } catch (const std::exception &e) {
    std::cerr << "[HighscoreManager] Error loading highscores: " << e.what() << std::endl;
    m_highscores.clear();
    return false;
  }
}

bool HighscoreManager::saveToFile() const
{
  try {
    const fs::path filePath = getHighscoreFilePath();

    // Create directory if it doesn't exist
    fs::create_directories(filePath.parent_path());

    std::ofstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "[HighscoreManager] Failed to open highscore file for writing: " << filePath << std::endl;
      return false;
    }

    nlohmann::json json = m_highscores;
    file << json.dump(2);  // Pretty print with 2-space indentation

    return true;
  } catch (const std::exception &e) {
    std::cerr << "[HighscoreManager] Error saving highscores: " << e.what() << std::endl;
    return false;
  }
}

bool HighscoreManager::addHighscore(const HighscoreEntry &entry)
{
  // Check if this score qualifies for the highscore list
  if (!isHighscore(entry.score)) {
    return false;
  }

  m_highscores.push_back(entry);
  sortHighscores();

  // Keep only top 5
  if (m_highscores.size() > MAX_HIGHSCORES) {
    m_highscores.resize(MAX_HIGHSCORES);
  }

  // Save to file
  return saveToFile();
}

bool HighscoreManager::isHighscore(int score) const
{
  // If we have less than 5 highscores, any score qualifies
  if (m_highscores.size() < MAX_HIGHSCORES) {
    return true;
  }

  // Check if score is higher than the lowest highscore
  return score > m_highscores.back().score;
}

void HighscoreManager::clearHighscores()
{
  m_highscores.clear();
  saveToFile();
}

void HighscoreManager::sortHighscores()
{
  std::sort(m_highscores.begin(), m_highscores.end(),
            [](const HighscoreEntry &a, const HighscoreEntry &b) {
              return a.score > b.score;  // Descending order
            });
}

fs::path HighscoreManager::getHighscoreFilePath() const
{
  // Store highscores in the user's home directory for persistence
  // This ensures it works regardless of where the executable is run from
  const char* homeDir = std::getenv("HOME");
  if (homeDir != nullptr) {
    return fs::path(homeDir) / ".r-type" / HIGHSCORE_FILE;
  }
  
  // Fallback to current directory
  return fs::current_path() / HIGHSCORE_FILE;
}
