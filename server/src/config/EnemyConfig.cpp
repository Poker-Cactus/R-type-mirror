/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EnemyConfig.cpp - Enemy configuration loader implementation
*/

#include "../include/config/EnemyConfig.hpp"
#include <fstream>
#include <iostream>

namespace server
{

bool EnemyConfigManager::loadFromFile(const std::string &filepath)
{
  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "[EnemyConfig] Failed to open config file: " << filepath << std::endl;
    return false;
  }

  try {
    nlohmann::json json;
    file >> json;

    if (!json.contains("enemies") || !json["enemies"].is_array()) {
      std::cerr << "[EnemyConfig] Invalid config format: missing 'enemies' array" << std::endl;
      return false;
    }

    m_configs.clear();

    for (const auto &enemyJson : json["enemies"]) {
      EnemyConfig config = EnemyConfig::fromJson(enemyJson);
      if (config.id.empty()) {
        std::cerr << "[EnemyConfig] Warning: skipping enemy with empty ID" << std::endl;
        continue;
      }
      m_configs[config.id] = config;
      std::cout << "[EnemyConfig] Loaded enemy: " << config.id << " (" << config.name << ")" << std::endl;
    }

    std::cout << "[EnemyConfig] Successfully loaded " << m_configs.size() << " enemy types from " << filepath
              << std::endl;
    return true;

  } catch (const nlohmann::json::exception &e) {
    std::cerr << "[EnemyConfig] JSON parsing error: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "[EnemyConfig] Error loading config: " << e.what() << std::endl;
    return false;
  }
}

const EnemyConfig *EnemyConfigManager::getConfig(const std::string &id) const
{
  auto it = m_configs.find(id);
  if (it != m_configs.end()) {
    return &it->second;
  }
  return nullptr;
}

std::vector<std::string> EnemyConfigManager::getEnemyIds() const
{
  std::vector<std::string> ids;
  ids.reserve(m_configs.size());
  for (const auto &[id, config] : m_configs) {
    ids.push_back(id);
  }
  return ids;
}

} // namespace server
