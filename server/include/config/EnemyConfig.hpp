/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EnemyConfig.hpp - Enemy configuration loader
*/

#ifndef SERVER_ENEMY_CONFIG_HPP_
#define SERVER_ENEMY_CONFIG_HPP_

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace server
{

/**
 * @brief Configuration structure for an enemy type
 */
struct EnemyConfig {
  std::string id;
  std::string name;

  // Sprite configuration
  struct SpriteConfig {
    uint32_t spriteId;
    uint32_t width;
    uint32_t height;
    bool animated;
    uint32_t frameCount;
    uint32_t startFrame;
    uint32_t endFrame;
    float frameTime;
    bool reverseAnimation;
  } sprite;

  // Transform configuration
  struct TransformConfig {
    float scale;
  } transform;

  // Health configuration
  struct HealthConfig {
    int hp;
    int maxHp;
  } health;

  // Collider configuration
  struct ColliderConfig {
    float width;
    float height;
  } collider;

  // Velocity configuration
  struct VelocityConfig {
    float dx;
    float dy;
  } velocity;

  // Pattern configuration
  struct PatternConfig {
    std::string type;
    float amplitude;
    float frequency;
  } pattern;

  // Spawn configuration
  struct SpawnConfig {
    int groupSizeMin;
    int groupSizeMax;
    float spawnDelay;
    float spawnInterval;
  } spawn;

  /**
   * @brief Parse EnemyConfig from JSON
   */
  static EnemyConfig fromJson(const nlohmann::json &json)
  {
    EnemyConfig config;
    config.id = json.value("id", "");
    config.name = json.value("name", "");

    if (json.contains("sprite")) {
      const auto &s = json["sprite"];
      config.sprite.spriteId = s.value("spriteId", 0u);
      config.sprite.width = s.value("width", 32u);
      config.sprite.height = s.value("height", 32u);
      config.sprite.animated = s.value("animated", false);
      config.sprite.frameCount = s.value("frameCount", 1u);
      config.sprite.startFrame = s.value("startFrame", 0u);
      config.sprite.endFrame = s.value("endFrame", 0u);
      config.sprite.frameTime = s.value("frameTime", 0.1f);
      config.sprite.reverseAnimation = s.value("reverseAnimation", false);
    }

    if (json.contains("transform")) {
      const auto &t = json["transform"];
      config.transform.scale = t.value("scale", 1.0f);
    }

    // Default collider to sprite dimensions (scaled) unless explicitly provided
    config.collider.width = static_cast<float>(config.sprite.width) * config.transform.scale;
    config.collider.height = static_cast<float>(config.sprite.height) * config.transform.scale;

    if (json.contains("health")) {
      const auto &h = json["health"];
      config.health.hp = h.value("hp", 10);
      config.health.maxHp = h.value("maxHp", 10);
    }

    if (json.contains("collider")) {
      const auto &c = json["collider"];
      config.collider.width = c.value("width", config.collider.width);
      config.collider.height = c.value("height", config.collider.height);
    }

    if (json.contains("velocity")) {
      const auto &v = json["velocity"];
      config.velocity.dx = v.value("dx", 0.0f);
      config.velocity.dy = v.value("dy", 0.0f);
    }

    if (json.contains("pattern")) {
      const auto &p = json["pattern"];
      config.pattern.type = p.value("type", "none");
      config.pattern.amplitude = p.value("amplitude", 0.0f);
      config.pattern.frequency = p.value("frequency", 0.0f);
    }

    if (json.contains("spawn")) {
      const auto &sp = json["spawn"];
      config.spawn.groupSizeMin = sp.value("groupSizeMin", 1);
      config.spawn.groupSizeMax = sp.value("groupSizeMax", 1);
      config.spawn.spawnDelay = sp.value("spawnDelay", 0.0f);
      config.spawn.spawnInterval = sp.value("spawnInterval", 5.0f);
    }

    return config;
  }
};

/**
 * @brief Manager for enemy configurations
 */
class EnemyConfigManager
{
public:
  /**
   * @brief Load enemy configurations from JSON file
   * @param filepath Path to the JSON configuration file
   * @return true if loaded successfully
   */
  bool loadFromFile(const std::string &filepath);

  /**
   * @brief Get enemy configuration by ID
   * @param id Enemy ID
   * @return Pointer to config or nullptr if not found
   */
  const EnemyConfig *getConfig(const std::string &id) const;

  /**
   * @brief Get all enemy configurations
   * @return Map of enemy ID to configuration
   */
  const std::unordered_map<std::string, EnemyConfig> &getAllConfigs() const { return m_configs; }

  /**
   * @brief Get list of all enemy IDs
   * @return Vector of enemy IDs
   */
  std::vector<std::string> getEnemyIds() const;

private:
  std::unordered_map<std::string, EnemyConfig> m_configs;
};

} // namespace server

#endif // SERVER_ENEMY_CONFIG_HPP_
