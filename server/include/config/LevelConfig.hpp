/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LevelConfig.hpp - Level wave configuration loader
*/

#ifndef SERVER_LEVEL_CONFIG_HPP_
#define SERVER_LEVEL_CONFIG_HPP_

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace server
{

/**
 * @brief Configuration for a single enemy spawn in a wave
 */
struct EnemySpawn {
  std::string enemyType;  // ID de l'ennemi (enemy_red, enemy_blue, etc.)
  float x;                // Position X (ou -1 pour aléatoire)
  float y;                // Position Y (ou -1 pour aléatoire)
  float delay;            // Délai avant spawn (en secondes depuis le début de la vague)
  int count;              // Nombre d'ennemis à spawner (défaut: 1)
  float spacing;          // Espacement entre les ennemis du groupe (défaut: 50.0)

  static EnemySpawn fromJson(const nlohmann::json &json)
  {
    EnemySpawn spawn;
    spawn.enemyType = json.value("enemyType", "enemy_red");
    spawn.x = json.value("x", -1.0f);
    spawn.y = json.value("y", -1.0f);
    spawn.delay = json.value("delay", 0.0f);
    spawn.count = json.value("count", 1);
    spawn.spacing = json.value("spacing", 50.0f);
    return spawn;
  }
};

/**
 * @brief Configuration for a wave of enemies
 */
struct WaveConfig {
  std::string id;
  std::string name;
  float startTime;                    // Temps de début de la vague (en secondes depuis le début du niveau)
  std::vector<EnemySpawn> spawns;     // Liste des ennemis à spawner dans cette vague

  static WaveConfig fromJson(const nlohmann::json &json)
  {
    WaveConfig wave;
    wave.id = json.value("id", "");
    wave.name = json.value("name", "");
    wave.startTime = json.value("startTime", 0.0f);

    if (json.contains("spawns") && json["spawns"].is_array()) {
      for (const auto &spawnJson : json["spawns"]) {
        wave.spawns.push_back(EnemySpawn::fromJson(spawnJson));
      }
    }

    return wave;
  }
};

/**
 * @brief Configuration for a complete level
 */
struct LevelConfig {
  std::string id;
  std::string name;
  std::string description;
  std::vector<WaveConfig> waves;

  static LevelConfig fromJson(const nlohmann::json &json)
  {
    LevelConfig level;
    level.id = json.value("id", "");
    level.name = json.value("name", "");
    level.description = json.value("description", "");

    if (json.contains("waves") && json["waves"].is_array()) {
      for (const auto &waveJson : json["waves"]) {
        level.waves.push_back(WaveConfig::fromJson(waveJson));
      }
    }

    return level;
  }
};

/**
 * @brief Manager for level configurations
 */
class LevelConfigManager
{
public:
  /**
   * @brief Load level configurations from JSON file
   * @param filepath Path to the JSON configuration file
   * @return true if loaded successfully
   */
  bool loadFromFile(const std::string &filepath);

  /**
   * @brief Get level configuration by ID
   * @param id Level ID
   * @return Pointer to config or nullptr if not found
   */
  const LevelConfig *getConfig(const std::string &id) const;

  /**
   * @brief Get all level configurations
   * @return Map of level ID to configuration
   */
  const std::unordered_map<std::string, LevelConfig> &getAllConfigs() const { return m_configs; }

  /**
   * @brief Get list of all level IDs
   * @return Vector of level IDs
   */
  std::vector<std::string> getLevelIds() const;

private:
  std::unordered_map<std::string, LevelConfig> m_configs;
};

} // namespace server

#endif // SERVER_LEVEL_CONFIG_HPP_
