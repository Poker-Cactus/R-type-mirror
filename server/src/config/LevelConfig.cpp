/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LevelConfig.cpp - Level configuration loader implementation
*/

#include "../include/config/LevelConfig.hpp"
#include <fstream>
#include <iostream>

namespace server
{

bool LevelConfigManager::loadFromFile(const std::string &filepath)
{
  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "[LevelConfig] Failed to open config file: " << filepath << std::endl;
    return false;
  }

  try {
    nlohmann::json json;
    file >> json;

    if (!json.contains("levels") || !json["levels"].is_array()) {
      std::cerr << "[LevelConfig] Invalid config format: missing 'levels' array" << std::endl;
      return false;
    }

    m_configs.clear();

    for (const auto &levelJson : json["levels"]) {
      LevelConfig config = LevelConfig::fromJson(levelJson);
      if (config.id.empty()) {
        std::cerr << "[LevelConfig] Warning: skipping level with empty ID" << std::endl;
        continue;
      }
      m_configs[config.id] = config;
      std::cout << "[LevelConfig] Loaded level: " << config.id << " (" << config.name << ") with " 
                << config.waves.size() << " waves" << std::endl;
    }

    std::cout << "[LevelConfig] Successfully loaded " << m_configs.size() << " levels from " << filepath
              << std::endl;
    return true;

  } catch (const nlohmann::json::exception &e) {
    std::cerr << "[LevelConfig] JSON parsing error: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "[LevelConfig] Error loading config: " << e.what() << std::endl;
    return false;
  }
}

const LevelConfig *LevelConfigManager::getConfig(const std::string &id) const
{
  auto it = m_configs.find(id);
  if (it != m_configs.end()) {
    return &it->second;
  }
  return nullptr;
}

std::vector<std::string> LevelConfigManager::getLevelIds() const
{
  std::vector<std::string> ids;
  ids.reserve(m_configs.size());
  for (const auto &[id, config] : m_configs) {
    ids.push_back(id);
  }
  return ids;
}

} // namespace server
