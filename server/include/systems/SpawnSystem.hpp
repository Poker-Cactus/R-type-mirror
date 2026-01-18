/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** SpawnSystem.hpp - Handles entity spawning
*/

#ifndef SERVER_SPAWN_SYSTEM_HPP_
#define SERVER_SPAWN_SYSTEM_HPP_

#include "../../../common/include/Common.hpp"

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/components/GunOffset.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/components/Immortal.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Shield.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/Viewport.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "../config/EnemyConfig.hpp"
#include "../config/LevelConfig.hpp"
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

namespace server
{

// Local enum for powerup types (not an ECS component)
enum class PowerupType { DRONE = 0, BUBBLE = 1, BUBBLE_TRIPLE = 2, BUBBLE_RUBAN = 3 };

typedef struct SpawnedProjectileConfig {
  float velocityX;
  float velocityY;
  float posX;
  float posY;
  ecs::Entity owner;
} SpawnedProjectileConfig;

/**
 * @brief System that handles entity spawning via events
 */
class SpawnSystem : public ecs::ISystem
{
public:
  SpawnSystem() : m_rng(std::random_device{}()) {}
  Difficulty difficulty = Difficulty::MEDIUM;

  /**
   * @brief Set the enemy configuration manager
   * @param configManager Shared pointer to enemy config manager
   */
  void setEnemyConfigManager(std::shared_ptr<EnemyConfigManager> configManager)
  {
    m_enemyConfigManager = configManager;
  }

  /**
   * @brief Set the level configuration manager
   * @param configManager Shared pointer to level config manager
   */
  void setLevelConfigManager(std::shared_ptr<LevelConfigManager> configManager)
  {
    m_levelConfigManager = configManager;
  }

  /**
   * @brief Set the current game mode for spawning
   * @param mode Game mode
   */
  void setGameMode(GameMode mode)
  {
    m_gameMode = mode;
    if (mode == GameMode::ENDLESS) {
      enableInfiniteMode();
    } else {
      disableInfiniteMode();
    }
  }

  /**
   * @brief Start a level by ID
   * @param levelId Level ID from configuration
   */
  void startLevel(const std::string &levelId)
  {
    if (!m_levelConfigManager) {
      std::cerr << "[SpawnSystem] ERROR: No level config manager set!" << std::endl;
      return;
    }

    const LevelConfig *config = m_levelConfigManager->getConfig(levelId);
    if (!config) {
      std::cerr << "[SpawnSystem] ERROR: Unknown level ID '" << levelId << "'" << std::endl;
      return;
    }

    m_currentLevel = config;
    m_levelTime = 0.0F;
    m_nextWaveIndex = 0;
    m_isLevelActive = true;

    // Clear existing spawn modes
    m_enemyTypeTimers.clear();
    m_spawnQueue.clear();

    std::cout << "[SpawnSystem] Started level: " << config->name << " (" << config->waves.size() << " waves)"
              << std::endl;
  }

  /**
   * @brief Stop the current level
   */
  void stopLevel()
  {
    m_isLevelActive = false;
    m_currentLevel = nullptr;
    m_levelTime = 0.0F;
    m_nextWaveIndex = 0;
    std::cout << "[SpawnSystem] Level stopped" << std::endl;
  }

  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    m_spawnTimer += deltaTime;
    m_powerupSpawnTimer += deltaTime;

    // Priority - Infinite mode
    if (m_isInfiniteMode) {
      updateInfiniteSpawning(world, deltaTime);
      processSpawnQueue(world, deltaTime);

      if (m_powerupSpawnTimer >= POWERUP_SPAWN_INTERVAL) {
        spawnPowerupRandom(world);
        m_powerupSpawnTimer = 0.0F;
      }
      return;
    }

    // Priority 0: Level-based spawning (highest priority)
    if (m_isLevelActive && m_currentLevel) {
      updateLevelSpawning(world, deltaTime);
      // Still process spawn queue even in level mode
      processSpawnQueue(world, deltaTime);

      // Spawn powerups periodically even in level mode
      if (m_powerupSpawnTimer >= POWERUP_SPAWN_INTERVAL) {
        spawnPowerupRandom(world);
        m_powerupSpawnTimer = 0.0F;
      }

      return;
    }

    // Update spawn queue timer for delayed spawns
    processSpawnQueue(world, deltaTime);

    // Mode 1: Multi-type spawning avec timers séparés (prioritaire)
    if (!m_enemyTypeTimers.empty()) {
      updateMultiTypeSpawning(world, deltaTime);

      // Spawn powerups periodically in multi-type mode
      if (m_powerupSpawnTimer >= POWERUP_SPAWN_INTERVAL) {
        spawnPowerupRandom(world);
        m_powerupSpawnTimer = 0.0F;
      }

      return; // Skip le mode single-type
    }

    // Mode 2: Single-type spawning avec cycle automatique
    m_spawnTimer += deltaTime;
    if (!m_enemyConfigManager) {
      std::cerr << "[SpawnSystem] ERROR: No enemy config manager, cannot spawn enemies" << std::endl;
      return;
    }

    // Use config-based spawning
    const EnemyConfig *config = m_enemyConfigManager->getConfig(m_currentEnemyType);
    if (!config) {
      std::cerr << "[SpawnSystem] ERROR: Unknown enemy type '" << m_currentEnemyType << "'" << std::endl;
      return;
    }

    if (m_spawnTimer >= config->spawn.spawnInterval) {
      spawnEnemyGroup(world, m_currentEnemyType);
      m_spawnTimer = 0.0F;

      // Alterner automatiquement entre les types d'ennemis
      cycleEnemyType();
    }

    // Spawn powerups periodically
    if (m_powerupSpawnTimer >= POWERUP_SPAWN_INTERVAL) {
      spawnPowerupRandom(world);
      m_powerupSpawnTimer = 0.0F;
    }
  }

  void initialize(ecs::World &world)
  {
    // Subscribe to spawn events
    m_spawnHandle = world.subscribeEvent<ecs::SpawnEntityEvent>(
      [this, &world](const ecs::SpawnEntityEvent &event) { this->handleSpawnEvent(world, event); });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

  /**
   * @brief Set the current enemy type to spawn
   * @param enemyType Enemy type ID from config
   */
  void setCurrentEnemyType(const std::string &enemyType) { m_currentEnemyType = enemyType; }

  /**
   * @brief Get the current enemy type being spawned
   * @return Current enemy type ID
   */
  [[nodiscard]] const std::string &getCurrentEnemyType() const { return m_currentEnemyType; }

  /**
   * @brief Cycle to the next enemy type (useful for wave progression)
   */
  void cycleEnemyType()
  {
    if (!m_enemyConfigManager)
      return;

    auto ids = m_enemyConfigManager->getEnemyIds();
    if (ids.empty())
      return;

    auto it = std::find(ids.begin(), ids.end(), m_currentEnemyType);
    if (it != ids.end()) {
      ++it;
      if (it == ids.end()) {
        it = ids.begin(); // Loop back
      }
      m_currentEnemyType = *it;
    } else {
      m_currentEnemyType = ids[0];
    }
  }

  /**
   * @brief Spawn multiple enemy types at once
   * @param world ECS world
   * @param enemyTypes Vector of enemy type IDs to spawn
   */
  void spawnMultipleTypes(ecs::World &world, const std::vector<std::string> &enemyTypes)
  {
    for (const auto &enemyType : enemyTypes) {
      spawnEnemyGroup(world, enemyType);
    }
  }

  /**
   * @brief Enable spawning for specific enemy types with their own timers
   * @param enemyTypes Vector of enemy type IDs to enable
   */
  void enableMultipleSpawnTypes(const std::vector<std::string> &enemyTypes)
  {
    m_enemyTypeTimers.clear();
    for (const auto &type : enemyTypes) {
      m_enemyTypeTimers[type] = 0.0F;
      std::cout << "[SpawnSystem] Enabled multi-spawn for enemy type: " << type << std::endl;
    }
    std::cout << "[SpawnSystem] Multi-spawn mode activated with " << m_enemyTypeTimers.size() << " enemy types"
              << std::endl;
  }

  /**
   * @brief Update function that handles multiple enemy types spawning independently
   */
  void updateMultiTypeSpawning(ecs::World &world, float deltaTime)
  {
    if (!m_enemyConfigManager || m_enemyTypeTimers.empty())
      return;

    for (auto &[enemyType, timer] : m_enemyTypeTimers) {
      timer += deltaTime;

      const EnemyConfig *config = m_enemyConfigManager->getConfig(enemyType);
      if (config && timer >= config->spawn.spawnInterval) {
        std::cout << "[SpawnSystem] Spawning group of " << enemyType << " (timer=" << timer
                  << ", interval=" << config->spawn.spawnInterval << ")" << std::endl;
        spawnEnemyGroup(world, enemyType);
        timer = 0.0F;
      }
    }
  }

  /**
   * @brief Update function for infinite mode spawning
   */
  void updateInfiniteSpawning(ecs::World &world, float deltaTime)
  {
    if (!m_enemyConfigManager)
      return;

    m_infiniteElapsed += deltaTime;

    if (m_infiniteEnemyTypes.empty()) {
      m_infiniteEnemyTypes = m_enemyConfigManager->getEnemyIds();
      if (!m_infiniteEnemyTypes.empty()) {
        m_infiniteUnlockedCount = 1;
        m_infiniteEnemyTimers[m_infiniteEnemyTypes[0]] = 0.0F;
      }
    }

    if (m_infiniteUnlockedCount < m_infiniteEnemyTypes.size()) {
      m_infiniteUnlockTimer += deltaTime;
      if (m_infiniteUnlockTimer >= INFINITE_UNLOCK_INTERVAL) {
        m_infiniteUnlockTimer = 0.0F;
        const auto &newType = m_infiniteEnemyTypes[m_infiniteUnlockedCount];
        m_infiniteEnemyTimers[newType] = 0.0F;
        m_infiniteUnlockedCount++;
        std::cout << "[SpawnSystem] Infinite mode unlocked enemy type: " << newType << std::endl;
      }
    }

    const float ramp = 1.0f + (m_infiniteElapsed / INFINITE_RAMP_INTERVAL);
    int extraGroups = static_cast<int>(m_infiniteElapsed / INFINITE_EXTRA_GROUP_INTERVAL);
    if (extraGroups > INFINITE_MAX_EXTRA_GROUPS)
      extraGroups = INFINITE_MAX_EXTRA_GROUPS;

    for (auto &[enemyType, timer] : m_infiniteEnemyTimers) {
      timer += deltaTime;

      const EnemyConfig *config = m_enemyConfigManager->getConfig(enemyType);
      if (!config)
        continue;

      const float effectiveInterval = std::max(config->spawn.spawnInterval / ramp, INFINITE_MIN_INTERVAL);

      if (timer >= effectiveInterval) {
        spawnEnemyGroup(world, enemyType);
        for (int i = 0; i < extraGroups; ++i) {
          spawnEnemyGroup(world, enemyType);
        }
        timer = 0.0F;
      }
    }
  }

  void enableInfiniteMode()
  {
    m_isInfiniteMode = true;
    m_infiniteElapsed = 0.0F;
    m_infiniteUnlockTimer = 0.0F;
    m_infiniteUnlockedCount = 0;
    m_infiniteEnemyTimers.clear();
    m_infiniteEnemyTypes.clear();
    m_spawnQueue.clear();
    m_spawnQueueTimer = 0.0F;
    m_isLevelActive = false;
    m_currentLevel = nullptr;
    m_enemyTypeTimers.clear();
  }

  void disableInfiniteMode()
  {
    m_isInfiniteMode = false;
    m_infiniteElapsed = 0.0F;
    m_infiniteUnlockTimer = 0.0F;
    m_infiniteUnlockedCount = 0;
    m_infiniteEnemyTimers.clear();
    m_infiniteEnemyTypes.clear();
  }

  /**
   * @brief Process the spawn queue for delayed spawns
   */
  void processSpawnQueue(ecs::World &world, float deltaTime)
  {
    if (m_spawnQueue.empty())
      return;

    m_spawnQueueTimer += deltaTime;

    while (!m_spawnQueue.empty() && m_spawnQueueTimer >= m_spawnQueue.front().delay) {
      const auto &queuedSpawn = m_spawnQueue.front();
      if (!m_enemyConfigManager) {
        std::cerr << "[SpawnSystem] CRITICAL: No enemy config manager set!" << std::endl;
        m_spawnQueue.erase(m_spawnQueue.begin());
        continue;
      }
      if (queuedSpawn.enemyType.empty()) {
        std::cerr << "[SpawnSystem] ERROR: Empty enemy type in spawn queue!" << std::endl;
        m_spawnQueue.erase(m_spawnQueue.begin());
        continue;
      }

      // Spawn single enemy (count should always be 1 now)
      spawnEnemyFromConfig(world, queuedSpawn.x, queuedSpawn.y, queuedSpawn.enemyType);

      m_spawnQueueTimer -= queuedSpawn.delay;
      m_spawnQueue.erase(m_spawnQueue.begin());
    }
  }

  /**
   * @brief Update level spawning - processes waves based on level time
   */
  void updateLevelSpawning(ecs::World &world, float deltaTime)
  {
    if (!m_currentLevel || !m_isLevelActive)
      return;

    m_levelTime += deltaTime;

    // Check if we need to trigger the next wave
    while (m_nextWaveIndex < m_currentLevel->waves.size()) {
      const auto &wave = m_currentLevel->waves[m_nextWaveIndex];

      if (m_levelTime >= wave.startTime) {
        std::cout << "[SpawnSystem] Triggering wave " << m_nextWaveIndex << ": " << wave.name
                  << " (time=" << m_levelTime << ")" << std::endl;

        // Get viewport width to spawn just outside screen
        float worldWidth = DEFAULT_VIEWPORT_WIDTH;
        ecs::ComponentSignature playerSig;
        playerSig.set(ecs::getComponentId<ecs::PlayerId>());
        playerSig.set(ecs::getComponentId<ecs::Viewport>());
        std::vector<ecs::Entity> players;
        world.getEntitiesWithSignature(playerSig, players);
        for (const auto &player : players) {
          const auto &viewport = world.getComponent<ecs::Viewport>(player);
          if (viewport.width > 0) {
            worldWidth = std::max(worldWidth, static_cast<float>(viewport.width));
          }
        }

        // Queue all spawns in this wave
        // If count > 1, create individual spawns with incrementing delays
        for (const auto &spawn : wave.spawns) {
          // Get enemy config to calculate proper delay based on velocity
          const EnemyConfig *enemyConfig =
            m_enemyConfigManager ? m_enemyConfigManager->getConfig(spawn.enemyType) : nullptr;
          float enemyVelocity = enemyConfig ? std::abs(enemyConfig->velocity.dx) : 384.0f; // Default velocity

          // Calculate delay to maintain spacing: delay = spacing / velocity
          float spawnDelayPerEnemy = (enemyVelocity > 0.0f) ? (spawn.spacing / enemyVelocity) : 0.08f;

          std::uniform_real_distribution<float> yVariation(-30.0f, 30.0f);
          for (int i = 0; i < spawn.count; ++i) {
            float offsetY = yVariation(m_rng);
            float individualDelay = spawn.delay + i * spawnDelayPerEnemy;

            // Spawn just outside the right edge of screen (worldWidth + 100px)
            // Don't add offsetX here - let the delay create natural spacing
            float spawnX = worldWidth + 100.0f;

            m_spawnQueue.push_back({spawnX, spawn.y + offsetY, individualDelay, spawn.enemyType,
                                    1, // Spawn only 1 enemy per queue entry
                                    0.0f});
          }
        }

        // Sort spawn queue by delay
        std::sort(m_spawnQueue.begin(), m_spawnQueue.end(),
                  [](const QueuedSpawn &a, const QueuedSpawn &b) { return a.delay < b.delay; });

        m_spawnQueueTimer = 0.0F;
        m_nextWaveIndex++;
      } else {
        break; // No more waves to trigger yet
      }
    }

    // Check if level is complete
    if (m_nextWaveIndex >= m_currentLevel->waves.size() && m_spawnQueue.empty()) {
      std::cout << "[SpawnSystem] Level completed: " << m_currentLevel->name << std::endl;
      stopLevel();
    }
  }

private:
  ecs::EventListenerHandle m_spawnHandle;
  std::mt19937 m_rng;
  float m_spawnTimer = 0.0F;
  float m_spawnQueueTimer = 0.0F;
  float m_powerupSpawnTimer = 0.0F;
  int m_powerupSpawnCount = 0; // Counter to alternate powerup types: 0=DRONE, 1=BUBBLE
  std::shared_ptr<EnemyConfigManager> m_enemyConfigManager;
  std::shared_ptr<LevelConfigManager> m_levelConfigManager;
  std::string m_currentEnemyType = "enemy_red"; // Default enemy type

  GameMode m_gameMode = GameMode::CLASSIC;
  bool m_isInfiniteMode = false;
  float m_infiniteElapsed = 0.0F;
  float m_infiniteUnlockTimer = 0.0F;
  size_t m_infiniteUnlockedCount = 0;
  std::vector<std::string> m_infiniteEnemyTypes;
  std::unordered_map<std::string, float> m_infiniteEnemyTimers;

  // Level-based spawning state
  const LevelConfig *m_currentLevel = nullptr;
  float m_levelTime = 0.0F;
  size_t m_nextWaveIndex = 0;
  bool m_isLevelActive = false;

  // Timers individuels pour chaque type d'ennemi
  std::unordered_map<std::string, float> m_enemyTypeTimers;

  struct QueuedSpawn {
    float x;
    float y;
    float delay;
    std::string enemyType;
    int count; // Nombre d'ennemis à spawner
    float spacing; // Espacement entre les ennemis
  };
  std::vector<QueuedSpawn> m_spawnQueue;

  // Spawn configuration constants
  static constexpr float SPAWN_INTERVAL = 6.0F;
  static constexpr float POWERUP_SPAWN_INTERVAL = 15.0F; // Spawn powerup every 15 seconds
  static constexpr float DEFAULT_VIEWPORT_WIDTH = 800.0F;
  static constexpr float DEFAULT_VIEWPORT_HEIGHT = 600.0F;
  static constexpr float SPAWN_Y_MARGIN = 50.0F;
  static constexpr float SPAWN_X_OFFSET = 32.0F;
  static constexpr float ENEMY_VELOCITY_X = -384.0F;
  static constexpr int ENEMY_HEALTH = 30;
  static constexpr float ENEMY_COLLIDER_SIZE = 48.0F;
  static constexpr unsigned int ENEMY_SPRITE_SIZE = 96;

  // Infinite mode tuning
  static constexpr float INFINITE_UNLOCK_INTERVAL = 25.0F;
  static constexpr float INFINITE_RAMP_INTERVAL = 60.0F;
  static constexpr float INFINITE_MIN_INTERVAL = 0.7F;
  static constexpr float INFINITE_EXTRA_GROUP_INTERVAL = 45.0F;
  static constexpr int INFINITE_MAX_EXTRA_GROUPS = 2;

  // Enemy Red configuration
  static constexpr float ENEMY_RED_AMPLITUDE = 40.0F;
  static constexpr float ENEMY_RED_FREQUENCY = 6.0F;
  static constexpr float ENEMY_RED_SPAWN_DELAY = 0.3F; // Delay between each enemy in a group
  static constexpr float ENEMY_RED_SCALE = 3.0F;
  // Enemy Red sprite: 533x36 spritesheet with 16 frames
  static constexpr int ENEMY_RED_SPRITE_SHEET_WIDTH = 533;
  static constexpr int ENEMY_RED_SPRITE_HEIGHT = 36;
  static constexpr int ENEMY_RED_FRAME_COUNT = 16;
  static constexpr int ENEMY_RED_FRAME_WIDTH =
    ENEMY_RED_SPRITE_SHEET_WIDTH / ENEMY_RED_FRAME_COUNT; // 33px (integer division)

  // R-Type_Items.png: 84x12 total with 7 frames, but we only use first 4
  // Frame 0=BUBBLE, Frame 1=BUBBLE_TRIPLE, Frame 2=BUBBLE_RUBAN, Frame 3=DRONE
  static constexpr int POWERUP_SPRITE_SHEET_WIDTH = 84;
  static constexpr int POWERUP_SPRITE_HEIGHT = 12;
  static constexpr int POWERUP_TOTAL_FRAMES = 7; // Total frames in texture
  static constexpr int POWERUP_FRAME_COUNT = 4; // Frames we actually use
  static constexpr int POWERUP_FRAME_WIDTH = POWERUP_SPRITE_SHEET_WIDTH / POWERUP_TOTAL_FRAMES; // 12px per frame
  static constexpr float POWERUP_VELOCITY_X = -100.0F; // Slow drift left
  static constexpr float POWERUP_COLLIDER_SIZE = 32.0F;
  static constexpr float POWERUP_SCALE = 4.0F;

  static constexpr float PROJECTILE_COLLIDER_SIZE = 8.0F;
  static constexpr unsigned int PROJECTILE_SPRITE_WIDTH = 84;
  static constexpr unsigned int PROJECTILE_SPRITE_HEIGHT = 37;
  static constexpr float PROJECTILE_VELOCITY_MULTIPLIER = 2400.0F;
  static constexpr float DIRECTION_THRESHOLD = 0.01F;

  // Charged projectile configuration
  static constexpr float CHARGED_PROJECTILE_COLLIDER_SIZE = 20.0F;
  static constexpr unsigned int CHARGED_PROJECTILE_SPRITE_WIDTH = 165;
  static constexpr unsigned int CHARGED_PROJECTILE_SPRITE_HEIGHT = 16;
  static constexpr float CHARGED_PROJECTILE_VELOCITY = 2400.0F;
  static constexpr float CHARGED_PROJECTILE_SCALE = 3.0F;

  // loading shot configuration
  static constexpr float LOADING_SHOT_COLLIDER_SIZE = 12.0F;
  static constexpr unsigned int LOADING_SHOT_SPRITE_WIDTH = 255 / 8;
  static constexpr unsigned int LOADING_SHOT_SPRITE_HEIGHT = 29;
  static constexpr float LOADING_SHOT_VELOCITY = 0.0F;
  static constexpr float LOADING_SHOT_SCALE = 2.5F;
  static constexpr float LOADING_SHOT_FRAME_TIME = 0.12F; // plus rapide (≈1s pour 8 frames)

  // Ruban/Wave beam projectile configuration (R-Type ribbon effect)
  // Uses xruban_projectile.png format (x = phase 1-14)
  // Phase 1 initial dimensions: 21x49, 1 frame
  static constexpr float RUBAN_PROJECTILE_VELOCITY = 1800.0F; // Slower than regular shot (2400.0F)
  static constexpr float RUBAN_WAVE_AMPLITUDE = 50.0F;
  static constexpr float RUBAN_WAVE_FREQUENCY = 12.0F;
  static constexpr float RUBAN_SCALE = 3.0F;
  static constexpr unsigned int RUBAN_INITIAL_WIDTH = 21;
  static constexpr unsigned int RUBAN_INITIAL_HEIGHT = 49;

  typedef struct configRubanProjectile {
    unsigned int spriteWidth;
    unsigned int spriteHeight;
    unsigned int totalFrames;
    float frameWidth;
    float scale;
    std::uint32_t spriteId;
  } configRubanProjectile;

  void spawnFollower(ecs::World &world, ecs::Entity parent, ecs::Entity child, float offsetX, float offsetY)
  {
    ecs::Follower follower;
    follower.parent = parent;
    follower.offsetX = offsetX;
    follower.offsetY = offsetY;
    world.addComponent(child, follower);
  }

  /**
   * @brief Spawn a powerup at a random position on the right side of the screen
   */
  void spawnPowerupRandom(ecs::World &world)
  {
    // Get world dimensions
    float worldWidth = DEFAULT_VIEWPORT_WIDTH;
    float worldHeight = DEFAULT_VIEWPORT_HEIGHT;

    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    playerSig.set(ecs::getComponentId<ecs::Viewport>());
    std::vector<ecs::Entity> players;
    world.getEntitiesWithSignature(playerSig, players);

    for (const auto &player : players) {
      if (world.hasComponent<ecs::Viewport>(player)) {
        const auto &viewport = world.getComponent<ecs::Viewport>(player);
        if (viewport.width > 0) {
          worldWidth = std::max(worldWidth, static_cast<float>(viewport.width));
        }
        if (viewport.height > 0) {
          worldHeight = std::max(worldHeight, static_cast<float>(viewport.height));
        }
      }
    }

    // Random Y position
    std::uniform_real_distribution<float> yDist(SPAWN_Y_MARGIN, worldHeight - SPAWN_Y_MARGIN);
    float spawnX = worldWidth - SPAWN_X_OFFSET;
    float spawnY = yDist(m_rng);

    // Alternate powerup types: DRONE first, then BUBBLE
    PowerupType powerupType;
    int cycle = m_powerupSpawnCount % 4;
    if (cycle == 0) {
      powerupType = PowerupType::DRONE;
    } else if (cycle == 1) {
      powerupType = PowerupType::BUBBLE;
    } else if (cycle == 2) {
      powerupType = PowerupType::BUBBLE_TRIPLE;
    } else {
      powerupType = PowerupType::BUBBLE_RUBAN;
    }
    m_powerupSpawnCount++;
    spawnPowerup(world, spawnX, spawnY, powerupType);
  }

  /**
   * @brief Spawn a group of enemies from configuration
   * @param world ECS world
   * @param enemyType Enemy type ID from config
   */
  void spawnEnemyGroup(ecs::World &world, const std::string &enemyType)
  {
    if (!m_enemyConfigManager) {
      std::cerr << "[SpawnSystem] CRITICAL: No enemy config manager set!" << std::endl;
      return;
    }

    const EnemyConfig *config = m_enemyConfigManager->getConfig(enemyType);
    if (!config) {
      std::cerr << "[SpawnSystem] ERROR: Unknown enemy type '" << enemyType << "'" << std::endl;
      std::cerr << "[SpawnSystem] Available enemy types: ";
      for (const auto &id : m_enemyConfigManager->getEnemyIds()) {
        std::cerr << id << " ";
      }
      std::cerr << std::endl;
      return;
    }

    // Get world dimensions
    float worldWidth = DEFAULT_VIEWPORT_WIDTH;
    float worldHeight = DEFAULT_VIEWPORT_HEIGHT;

    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    playerSig.set(ecs::getComponentId<ecs::Viewport>());
    std::vector<ecs::Entity> players;
    world.getEntitiesWithSignature(playerSig, players);

    for (const auto &player : players) {
      const auto &viewport = world.getComponent<ecs::Viewport>(player);
      if (viewport.width > 0) {
        worldWidth = std::max(worldWidth, static_cast<float>(viewport.width));
      }
      if (viewport.height > 0) {
        worldHeight = std::max(worldHeight, static_cast<float>(viewport.height));
      }
    }

    // Random group size from config
    std::uniform_int_distribution<int> groupSizeDist(config->spawn.groupSizeMin, config->spawn.groupSizeMax);
    int groupSize = groupSizeDist(m_rng);

    // Pick a random height range (1-6)
    std::uniform_int_distribution<int> heightRangeDist(0, 5);
    int heightRange = heightRangeDist(m_rng);

    // Calculate Y position within the chosen range (height / 6)
    float rangeHeight = (worldHeight - 2 * SPAWN_Y_MARGIN) / 6.0f;
    float baseY = SPAWN_Y_MARGIN + heightRange * rangeHeight;

    std::uniform_real_distribution<float> yOffsetDist(0.0f, rangeHeight);

    // Spawn X position at right edge
    float spawnX = worldWidth - SPAWN_X_OFFSET;

    // Queue all enemies in the group with delays
    for (int i = 0; i < groupSize; ++i) {
      float y = baseY + yOffsetDist(m_rng);
      float delay = i * config->spawn.spawnDelay;
      m_spawnQueue.push_back({spawnX, y, delay, enemyType, 1, 0.0F});
    }

    std::cout << "[SpawnSystem] Queued " << groupSize << " enemies of type '" << enemyType << "' at X=" << spawnX
              << std::endl;
  }

  /**
   * @brief Spawn an enemy from configuration
   * @param world ECS world
   * @param posX X position
   * @param posY Y position
   * @param enemyType Enemy type ID from config
   */
  void spawnEnemyFromConfig(ecs::World &world, float posX, float posY, const std::string &enemyType)
  {
    if (!m_enemyConfigManager) {
      std::cerr << "[SpawnSystem] CRITICAL: No enemy config manager set!" << std::endl;
      return;
    }

    const EnemyConfig *config = m_enemyConfigManager->getConfig(enemyType);
    if (!config) {
      std::cerr << "[SpawnSystem] ERROR: Unknown enemy type '" << enemyType << "'" << std::endl;
      std::cerr << "[SpawnSystem] Available enemy types: ";
      for (const auto &id : m_enemyConfigManager->getEnemyIds()) {
        std::cerr << id << " ";
      }
      std::cerr << std::endl;
      return;
    }

    ecs::Entity enemy = world.createEntity();

    // Pattern component
    world.addComponent(enemy, ecs::Pattern{config->pattern.type, config->pattern.amplitude, config->pattern.frequency});

    // Transform component
    ecs::Transform transform;
    transform.x = posX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = config->transform.scale;
    world.addComponent(enemy, transform);

    // Velocity component
    ecs::Velocity velocity;
    velocity.dx = config->velocity.dx;
    velocity.dy = config->velocity.dy;
    world.addComponent(enemy, velocity);

    // Health component
    ecs::Health health;
    health.hp = config->health.hp;
    health.maxHp = config->health.maxHp;
    world.addComponent(enemy, health);

    // Collider component
    world.addComponent(enemy, ecs::Collider{config->collider.width, config->collider.height});

    // Sprite component
    ecs::Sprite sprite;
    sprite.spriteId = config->sprite.spriteId;
    sprite.width = config->sprite.width;
    sprite.height = config->sprite.height;
    sprite.animated = config->sprite.animated;
    sprite.frameCount = config->sprite.frameCount;
    sprite.startFrame = config->sprite.startFrame;
    sprite.endFrame = config->sprite.endFrame;
    sprite.currentFrame = config->sprite.startFrame;
    sprite.frameTime = config->sprite.frameTime;
    sprite.reverseAnimation = config->sprite.reverseAnimation;
    world.addComponent(enemy, sprite);

    // Elite enemy: spawn shield and make elite immortal until shield is destroyed
    if (enemyType == "enemy_elite_blue") {
      ecs::Immortal immortal;
      immortal.isImmortal = true;
      world.addComponent(enemy, immortal);
      spawnEliteShield(world, enemy, transform);
    }

    std::cout << "[SpawnSystem] Spawned enemy '" << enemyType << "' (spriteId=" << config->sprite.spriteId
              << ", pattern=" << config->pattern.type << ") at (" << posX << ", " << posY << ")" << std::endl;

    // Networked component
    ecs::Networked net;
    net.networkId = enemy;
    world.addComponent(enemy, net);
  }

  void spawnEliteShield(ecs::World &world, ecs::Entity parent, const ecs::Transform &parentTransform)
  {
    ecs::Entity shield = world.createEntity();

    // Shield transform (follow parent)
    ecs::Transform shieldTransform;
    shieldTransform.x = parentTransform.x;
    shieldTransform.y = parentTransform.y;
    shieldTransform.rotation = 0.0F;
    shieldTransform.scale = 2.5F;
    world.addComponent(shield, shieldTransform);

    ecs::Follower follower;
    follower.parent = parent;
    follower.offsetX = -60.0F;
    follower.offsetY = 20.0F;
    follower.smoothing = 30.0F;
    world.addComponent(shield, follower);

    // Shield marker with parent link
    ecs::Shield shieldComp;
    shieldComp.parent = parent;
    world.addComponent(shield, shieldComp);

    // Shield health: 3 hits (damageFromProjectile=20)
    ecs::Health shieldHealth;
    shieldHealth.hp = 60;
    shieldHealth.maxHp = 60;
    world.addComponent(shield, shieldHealth);

    // Shield collider (bubble frame size * scale)
    world.addComponent(shield, ecs::Collider{60.0F, 60.0F});

    // Shield sprite (match BUBBLE animation)
    ecs::Sprite shieldSprite;
    shieldSprite.spriteId = ecs::SpriteId::SHIELD_BUBBLE;
    shieldSprite.width = 24; // bubble.png frame width
    shieldSprite.height = 24; // bubble.png height
    shieldSprite.animated = true;
    shieldSprite.frameCount = 12;
    shieldSprite.startFrame = 0;
    shieldSprite.endFrame = 11;
    shieldSprite.currentFrame = 0;
    shieldSprite.frameTime = 0.1F;
    shieldSprite.reverseAnimation = false;
    shieldSprite.loop = true;
    shieldSprite.row = 0;
    shieldSprite.offsetX = 0;
    world.addComponent(shield, shieldSprite);

    // Networked for replication
    ecs::Networked net;
    net.networkId = shield;
    world.addComponent(shield, net);

    std::cout << "[SpawnSystem] Spawned elite shield for entity " << parent << " (shield=" << shield << ")"
              << std::endl;
  }

  static void handleSpawnEvent(ecs::World &world, const ecs::SpawnEntityEvent &event)
  {
    configRubanProjectile config;
    switch (event.type) {
    case ecs::SpawnEntityEvent::EntityType::NONE:
      // NONE type means nothing to spawn (e.g., simple bubble doesn't shoot)
      break;
    case ecs::SpawnEntityEvent::EntityType::ENEMY:
      std::cerr << "[SpawnSystem] WARNING: SpawnEntityEvent for ENEMY is deprecated, use spawnEnemyFromConfig instead"
                << std::endl;
      break;
    case ecs::SpawnEntityEvent::EntityType::PROJECTILE:
      spawnProjectile(world, event.x, event.y, event.spawner);
      break;
    case ecs::SpawnEntityEvent::EntityType::CHARGED_PROJECTILE:
      spawnChargedProjectile(world, event.x, event.y, event.spawner);
      break;
    case ecs::SpawnEntityEvent::EntityType::LOADING_SHOT:
      spawnLoadingShot(world, event.x, event.y, event.spawner);
      break;
    case ecs::SpawnEntityEvent::EntityType::TRIPLE_PROJECTILE:
      spawnTripleProjectile(world, event.x, event.y, event.spawner);
      break;
    case ecs::SpawnEntityEvent::EntityType::RUBAN1_PROJECTILE:
    case ecs::SpawnEntityEvent::EntityType::RUBAN2_PROJECTILE:
    case ecs::SpawnEntityEvent::EntityType::RUBAN3_PROJECTILE:
    case ecs::SpawnEntityEvent::EntityType::RUBAN4_PROJECTILE:
    case ecs::SpawnEntityEvent::EntityType::RUBAN5_PROJECTILE:
      // All ruban projectiles start at phase 1 and animate through 14 phases
      spawnRubanProjectile(world, event.x, event.y, event.spawner, config);
      break;
    case ecs::SpawnEntityEvent::EntityType::POWERUP:
      spawnPowerup(world, event.x, event.y);
      break;
    case ecs::SpawnEntityEvent::EntityType::EXPLOSION:
      spawnExplosion(world, event.x, event.y);
      break;
    }
  }

  static void spawnRubanProjectile(ecs::World &world, float posX, float posY, ecs::Entity owner,
                                   configRubanProjectile config)
  {
    // Ignore passed config - always start at phase 1
    (void)config;

    const float projectileVelocity = RUBAN_PROJECTILE_VELOCITY;

    ecs::Entity projectile = world.createEntity();

    // Capability-based offset: use GunOffset if entity has it
    float offsetX = 0.0F;
    if (world.hasComponent<ecs::GunOffset>(owner)) {
      offsetX = world.getComponent<ecs::GunOffset>(owner).x * 1.0F;
    }

    ecs::Transform transform;
    transform.x = posX + offsetX;
    // Adjust Y position to align ruban projectile with regular projectile
    // Regular projectile: 84x37 at scale 1.0 = 84x37 effective size
    // Ruban projectile starts at: 21x49 at scale 3.0 = 63x147 effective size
    // Center vertically: (147 - 37) / 2 = 55 pixels up
    transform.y = posY - 55.0f;
    transform.rotation = 0.0F;
    transform.scale = RUBAN_SCALE;
    world.addComponent(projectile, transform);

    ecs::Velocity velocity;
    velocity.dx = projectileVelocity;
    velocity.dy = 0.0F;
    world.addComponent(projectile, velocity);

    // Add wave beam pattern for R-Type ribbon effect (oscillating vertically)
    world.addComponent(projectile, ecs::Pattern{"wave_beam", RUBAN_WAVE_AMPLITUDE, RUBAN_WAVE_FREQUENCY});

    // Collider based on initial phase dimensions
    world.addComponent(projectile,
                       ecs::Collider{static_cast<float>(RUBAN_INITIAL_WIDTH) * RUBAN_SCALE,
                                     static_cast<float>(RUBAN_INITIAL_HEIGHT) * RUBAN_SCALE});

    // Start with phase 1 sprite (1ruban_projectile.png: 21x49, 1 frame)
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::RUBAN1_PROJECTILE;
    sprite.width = RUBAN_INITIAL_WIDTH;
    sprite.height = RUBAN_INITIAL_HEIGHT;
    sprite.animated = false;
    sprite.frameCount = 1;
    sprite.loop = false;
    sprite.startFrame = 0;
    sprite.endFrame = 0;
    sprite.currentFrame = 0;

    world.addComponent(projectile, sprite);

    // Mark as networked so the snapshot system replicates it to clients.
    ecs::Networked net;
    net.networkId = projectile;
    world.addComponent(projectile, net);

    // Track owner to prevent self-damage
    ecs::Owner ownerComp;
    ownerComp.ownerId = owner;
    world.addComponent(projectile, ownerComp);
  }

  static void spawnTripleProjectile(ecs::World &world, float posX, float posY, ecs::Entity owner)
  {
    const float projectileVelocity = PROJECTILE_VELOCITY_MULTIPLIER * 1.0F;

    // Capability-based offset: use GunOffset if entity has it
    float offsetX = 0.0F;
    if (world.hasComponent<ecs::GunOffset>(owner)) {
      offsetX = world.getComponent<ecs::GunOffset>(owner).x * 1.0F;
    }

    // Three projectile angles: 0° (forward), -50° (up-forward), 50° (down-forward)
    // Note: In game coords, negative Y = up, positive Y = down
    constexpr float PI = 3.14159265358979323846F;
    const std::array<float, 3> angles = {0.0F, -50.0F * PI / 180.0F, 50.0F * PI / 180.0F};

    // Corresponding sprite IDs for each direction
    const std::array<std::uint32_t, 3> spriteIds = {
      ecs::SpriteId::TRIPLE_PROJECTILE_RIGHT, // Straight forward
      ecs::SpriteId::TRIPLE_PROJECTILE_UP, // Up-forward
      ecs::SpriteId::TRIPLE_PROJECTILE_DOWN // Down-forward
    };

    for (int i = 0; i < 3; i++) {
      ecs::Entity projectile = world.createEntity();

      ecs::Transform transform;
      transform.x = posX + offsetX;
      transform.y = posY;
      transform.rotation = 0.0F; // No rotation needed - sprite already oriented correctly
      transform.scale = 1.0F;
      world.addComponent(projectile, transform);

      ecs::Velocity velocity;
      velocity.dx = projectileVelocity * std::cos(angles[i]);
      velocity.dy = projectileVelocity * std::sin(angles[i]);
      world.addComponent(projectile, velocity);

      world.addComponent(projectile, ecs::Collider{PROJECTILE_COLLIDER_SIZE, PROJECTILE_COLLIDER_SIZE});

      // Sprite for triple projectile - use direction-specific sprite
      ecs::Sprite sprite;
      sprite.spriteId = spriteIds[i];
      sprite.width = PROJECTILE_SPRITE_WIDTH;
      sprite.height = PROJECTILE_SPRITE_HEIGHT;
      sprite.animated = false; // Individual images, no animation
      sprite.frameCount = 1;
      sprite.loop = false;
      sprite.startFrame = 0;
      sprite.endFrame = 0;

      world.addComponent(projectile, sprite);

      // Mark as networked
      ecs::Networked net;
      net.networkId = projectile;
      world.addComponent(projectile, net);

      // Track owner to prevent self-damage
      ecs::Owner ownerComp;
      ownerComp.ownerId = owner;
      world.addComponent(projectile, ownerComp);
    }
  }

  static void spawnProjectile(ecs::World &world, float posX, float posY, ecs::Entity owner)
  {
    // Determine direction: prefer requested direction, fall back to owner velocity
    float directionX = 0.0F;
    if (std::abs(directionX) < DIRECTION_THRESHOLD && world.hasComponent<ecs::Velocity>(owner)) {
      directionX = world.getComponent<ecs::Velocity>(owner).dx;
    }
    if (std::abs(directionX) < DIRECTION_THRESHOLD) {
      directionX = 1.0F; // Default to right if nothing else is known
    }

    const float projectileVelocity = PROJECTILE_VELOCITY_MULTIPLIER * 1.0F;

    ecs::Entity projectile = world.createEntity();

    // Capability-based offset: use GunOffset if entity has it (no identity checks).
    // Systems ask "What can this entity do?" not "What kind is it?"
    float offsetX = 0.0F;
    if (world.hasComponent<ecs::GunOffset>(owner)) {
      offsetX = world.getComponent<ecs::GunOffset>(owner).x * 1.0F;
    }

    ecs::Transform transform;
    transform.x = posX + offsetX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = 1.0F;
    world.addComponent(projectile, transform);

    ecs::Velocity velocity;
    velocity.dx = projectileVelocity;
    velocity.dy = 0.0F;
    world.addComponent(projectile, velocity);

    // Despawn is handled by LifetimeSystem when projectile leaves the viewport.

    world.addComponent(projectile, ecs::Collider{PROJECTILE_COLLIDER_SIZE, PROJECTILE_COLLIDER_SIZE});

    // SERVER ASSIGNS VISUAL IDENTITY AS DATA
    // Projectile sprite decided at creation, never inferred later
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::PROJECTILE;
    sprite.width = PROJECTILE_SPRITE_WIDTH; // 211x92 aspect ratio (422/2 frames, scaled down ~2.5x)
    sprite.height = PROJECTILE_SPRITE_HEIGHT;
    sprite.animated = true;
    sprite.frameCount = 3;
    sprite.loop = false;
    sprite.startFrame = 0;
    sprite.endFrame = 2;

    world.addComponent(projectile, sprite);

    // Mark as networked so the snapshot system replicates it to clients.
    ecs::Networked net;
    net.networkId = projectile;
    world.addComponent(projectile, net);

    // Track owner to prevent self-damage
    ecs::Owner ownerComp;
    ownerComp.ownerId = owner;
    world.addComponent(projectile, ownerComp);
  }

  static void spawnChargedProjectile(ecs::World &world, float posX, float posY, ecs::Entity owner)
  {
    float directionX = 0.0F;
    if (std::abs(directionX) < DIRECTION_THRESHOLD && world.hasComponent<ecs::Velocity>(owner)) {
      directionX = world.getComponent<ecs::Velocity>(owner).dx;
    }
    if (std::abs(directionX) < DIRECTION_THRESHOLD) {
      directionX = 1.0F;
    }

    ecs::Entity projectile = world.createEntity();

    float offsetX = 0.0F;
    if (world.hasComponent<ecs::GunOffset>(owner)) {
      offsetX = world.getComponent<ecs::GunOffset>(owner).x;
    }

    ecs::Transform transform;
    transform.x = posX + offsetX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = CHARGED_PROJECTILE_SCALE;
    world.addComponent(projectile, transform);

    ecs::Velocity velocity;
    velocity.dx = CHARGED_PROJECTILE_VELOCITY * 1.0F;
    velocity.dy = 0.0F;
    world.addComponent(projectile, velocity);

    world.addComponent(projectile, ecs::Collider{CHARGED_PROJECTILE_COLLIDER_SIZE, CHARGED_PROJECTILE_COLLIDER_SIZE});

    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::CHARGED_PROJECTILE;
    sprite.width = CHARGED_PROJECTILE_SPRITE_WIDTH;
    sprite.height = CHARGED_PROJECTILE_SPRITE_HEIGHT;
    sprite.animated = true;
    sprite.frameCount = 2;
    sprite.loop = true;
    sprite.startFrame = 0;
    sprite.endFrame = 1;
    world.addComponent(projectile, sprite);

    ecs::Networked net;
    net.networkId = projectile;
    world.addComponent(projectile, net);

    ecs::Owner ownerComp;
    ownerComp.ownerId = owner;
    world.addComponent(projectile, ownerComp);

    ecs::Immortal immortalComponent;
    immortalComponent.isImmortal = true;
    world.addComponent(projectile, immortalComponent);
  }

  static void spawnLoadingShot(ecs::World &world, float posX, float posY, ecs::Entity owner)
  {
    ecs::Entity loadingShot = world.createEntity();

    // Transform component
    ecs::Transform transform;
    transform.x = posX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = LOADING_SHOT_SCALE;
    world.addComponent(loadingShot, transform);

    // Velocity = 0 (l'animation suivra le joueur)
    ecs::Velocity velocity;
    velocity.dx = 0.0F;
    velocity.dy = 0.0F;
    world.addComponent(loadingShot, velocity);

    // Sprite avec animation de chargement
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::LOADING_SHOT;
    sprite.width = LOADING_SHOT_SPRITE_WIDTH;
    sprite.height = LOADING_SHOT_SPRITE_HEIGHT;
    sprite.animated = true;
    sprite.frameCount = 8;
    sprite.loop = true;
    sprite.startFrame = 0;
    sprite.endFrame = 7;
    sprite.frameTime = LOADING_SHOT_FRAME_TIME; // Animation rapide
    world.addComponent(loadingShot, sprite);

    // Networked component pour la réplication
    ecs::Networked net;
    net.networkId = loadingShot;
    world.addComponent(loadingShot, net);

    // Owner component pour lier au joueur
    ecs::Owner ownerComp;
    ownerComp.ownerId = owner;
    world.addComponent(loadingShot, ownerComp);

    std::cout << "[SpawnSystem] Spawned loading shot " << loadingShot << " for entity " << owner << std::endl;
  }

  static void spawnPowerup(ecs::World &world, float posX, float posY, PowerupType powerupType = PowerupType::DRONE)
  {
    ecs::Entity powerup = world.createEntity();

    // Transform
    ecs::Transform transform;
    transform.x = posX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = POWERUP_SCALE;
    world.addComponent(powerup, transform);

    // Velocity - slow drift to the left
    ecs::Velocity velocity;
    velocity.dx = POWERUP_VELOCITY_X;
    velocity.dy = 0.0F;
    world.addComponent(powerup, velocity);

    // Collider for pickup detection
    world.addComponent(powerup, ecs::Collider{POWERUP_COLLIDER_SIZE, POWERUP_COLLIDER_SIZE});

    // Sprite - all powerups use POWERUP spriteId with different currentFrame
    // Frame 0=BUBBLE, Frame 1=BUBBLE_TRIPLE, Frame 2=BUBBLE_RUBAN, Frame 3=DRONE
    // R-Type_Items.png: Frame 0=BUBBLE, Frame 1=BUBBLE_TRIPLE, Frame 2=BUBBLE_RUBAN, Frame 3=DRONE
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::POWERUP;
    sprite.width = POWERUP_FRAME_WIDTH;
    sprite.height = POWERUP_SPRITE_HEIGHT;
    sprite.animated = true; // Enable animation for cycling
    sprite.frameCount = POWERUP_FRAME_COUNT; // 4 frames total
    sprite.frameTime = 0.08f; // Animation speed
    sprite.loop = true;

    // Select starting frame and animation range based on powerup type
    switch (powerupType) {
    case PowerupType::BUBBLE:
      sprite.startFrame = 0;
      sprite.endFrame = 0;
      sprite.currentFrame = 0;
      break;
    case PowerupType::BUBBLE_TRIPLE:
      sprite.startFrame = 1;
      sprite.endFrame = 1;
      sprite.currentFrame = 1;
      break;
    case PowerupType::BUBBLE_RUBAN:
      sprite.startFrame = 2;
      sprite.endFrame = 2;
      sprite.currentFrame = 2;
      break;
    case PowerupType::DRONE:
    default:
      sprite.startFrame = 3;
      sprite.endFrame = 3;
      sprite.currentFrame = 3;
      break;
    }
    sprite.frameTime = 0.08f;
    sprite.loop = true;
    world.addComponent(powerup, sprite);

    // Networked for client replication
    ecs::Networked net;
    net.networkId = powerup;
    world.addComponent(powerup, net);

    const char *typeNames[] = {"DRONE", "BUBBLE", "BUBBLE_TRIPLE", "BUBBLE_RUBAN"};
    std::cout << "[SpawnSystem] Spawned " << typeNames[static_cast<int>(powerupType)] << " powerup at (" << posX << ", "
              << posY << ")\n";
  }

  static void spawnExplosion(ecs::World &world, float posX, float posY)
  {
    (void)world;
    (void)posX;
    (void)posY;
    // TODO: Implement explosion effect
  }
}; // namespace server

} // namespace server

#endif // SERVER_SPAWN_SYSTEM_HPP_
