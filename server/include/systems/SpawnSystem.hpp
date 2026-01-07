/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** SpawnSystem.hpp - Handles entity spawning
*/

#ifndef SERVER_SPAWN_SYSTEM_HPP_
#define SERVER_SPAWN_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/GunOffset.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/Viewport.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "../config/EnemyConfig.hpp"
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

namespace server
{

/**
 * @brief System that handles entity spawning via events
 */
class SpawnSystem : public ecs::ISystem
{
public:
  SpawnSystem() : m_rng(std::random_device{}()) {}

  /**
   * @brief Set the enemy configuration manager
   * @param configManager Shared pointer to enemy config manager
   */
  void setEnemyConfigManager(std::shared_ptr<EnemyConfigManager> configManager) { m_enemyConfigManager = configManager; }

  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;

    // Update spawn queue timer for delayed spawns
    if (!m_spawnQueue.empty()) {
      m_spawnQueueTimer += deltaTime;

      while (!m_spawnQueue.empty() && m_spawnQueueTimer >= m_spawnQueue.front().delay) {
        const auto &queuedSpawn = m_spawnQueue.front();
        // Use config-based spawning if available
        if (m_enemyConfigManager && !queuedSpawn.enemyType.empty()) {
          spawnEnemyFromConfig(world, queuedSpawn.x, queuedSpawn.y, queuedSpawn.enemyType);
        } else {
          spawnEnemyRed(world, queuedSpawn.x, queuedSpawn.y);
        }
        m_spawnQueueTimer -= queuedSpawn.delay;
        m_spawnQueue.erase(m_spawnQueue.begin());
      }
    }

    // Mode 1: Multi-type spawning avec timers séparés (prioritaire)
    if (!m_enemyTypeTimers.empty()) {
      updateMultiTypeSpawning(world, deltaTime);
      return; // Skip le mode single-type
    }

    // Mode 2: Single-type spawning avec cycle automatique
    m_spawnTimer += deltaTime;
    if (m_enemyConfigManager) {
      // Use config-based spawning
      const EnemyConfig *config = m_enemyConfigManager->getConfig(m_currentEnemyType);
      if (config && m_spawnTimer >= config->spawn.spawnInterval) {
        spawnEnemyGroup(world, m_currentEnemyType);
        m_spawnTimer = 0.0F;
        
        // Alterner automatiquement entre rouge et bleu
        cycleEnemyType();
      }
    } else if (m_spawnTimer >= SPAWN_INTERVAL) {
      // Fallback to hardcoded spawning
      spawnEnemyRedGroup(world);
      m_spawnTimer = 0.0F;
    }
  }

  void initialize(ecs::World &world)
  {
    // Subscribe to spawn events
    m_spawnHandle = world.subscribeEvent<ecs::SpawnEntityEvent>(
      [&world](const ecs::SpawnEntityEvent &event) { handleSpawnEvent(world, event); });
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
    }
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
        spawnEnemyGroup(world, enemyType);
        timer = 0.0F;
      }
    }
  }

private:
  ecs::EventListenerHandle m_spawnHandle;
  std::mt19937 m_rng;
  float m_spawnTimer = 0.0F;
  float m_spawnQueueTimer = 0.0F;
  std::shared_ptr<EnemyConfigManager> m_enemyConfigManager;
  std::string m_currentEnemyType = "enemy_red"; // Default enemy type

  // Timers individuels pour chaque type d'ennemi
  std::unordered_map<std::string, float> m_enemyTypeTimers;

  struct QueuedSpawn {
    float x;
    float y;
    float delay;
    std::string enemyType;
  };
  std::vector<QueuedSpawn> m_spawnQueue;

  // Spawn configuration constants
  static constexpr float SPAWN_INTERVAL = 6.0F;
  static constexpr float DEFAULT_VIEWPORT_WIDTH = 800.0F;
  static constexpr float DEFAULT_VIEWPORT_HEIGHT = 600.0F;
  static constexpr float SPAWN_Y_MARGIN = 50.0F;
  static constexpr float SPAWN_X_OFFSET = 32.0F;
  static constexpr float ENEMY_VELOCITY_X = -384.0F;
  static constexpr int ENEMY_HEALTH = 30;
  static constexpr float ENEMY_COLLIDER_SIZE = 48.0F;
  static constexpr unsigned int ENEMY_SPRITE_SIZE = 96;

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

  static constexpr float PROJECTILE_COLLIDER_SIZE = 8.0F;
  static constexpr unsigned int PROJECTILE_SPRITE_WIDTH = 84;
  static constexpr unsigned int PROJECTILE_SPRITE_HEIGHT = 37;
  static constexpr float PROJECTILE_VELOCITY_MULTIPLIER = 400.0F;
  static constexpr float DIRECTION_THRESHOLD = 0.01F;

  /**
   * @brief Spawn a group of enemies from configuration
   * @param world ECS world
   * @param enemyType Enemy type ID from config
   */
  void spawnEnemyGroup(ecs::World &world, const std::string &enemyType)
  {
    if (!m_enemyConfigManager) {
      spawnEnemyRedGroup(world);
      return;
    }

    const EnemyConfig *config = m_enemyConfigManager->getConfig(enemyType);
    if (!config) {
      std::cerr << "[SpawnSystem] Unknown enemy type: " << enemyType << std::endl;
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
      m_spawnQueue.push_back({spawnX, y, delay, enemyType});
    }
  }

  /**
   * @brief Spawn a group of Enemy Red with random count (1-5)
   */
  void spawnEnemyRedGroup(ecs::World &world)
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
      const auto &viewport = world.getComponent<ecs::Viewport>(player);
      if (viewport.width > 0) {
        worldWidth = std::max(worldWidth, static_cast<float>(viewport.width));
      }
      if (viewport.height > 0) {
        worldHeight = std::max(worldHeight, static_cast<float>(viewport.height));
      }
    }

    // Random group size: 1 to 5 enemies
    std::uniform_int_distribution<int> groupSizeDist(1, 5);
    int groupSize = groupSizeDist(m_rng);

    // Pick a random height range (1-6)
    std::uniform_int_distribution<int> heightRangeDist(0, 5);
    int heightRange = heightRangeDist(m_rng);

    // Calculate Y position within the chosen range (height / 6)
    float rangeHeight = (worldHeight - 2 * SPAWN_Y_MARGIN) / 6.0f;
    float baseY = SPAWN_Y_MARGIN + heightRange * rangeHeight;

    // Random Y within this range
    std::uniform_real_distribution<float> yOffsetDist(0.0f, rangeHeight);

    // Spawn X position at right edge
    float spawnX = worldWidth - SPAWN_X_OFFSET;

    // Queue all enemies in the group with delays
    for (int i = 0; i < groupSize; ++i) {
      float y = baseY + yOffsetDist(m_rng);
      float delay = i * ENEMY_RED_SPAWN_DELAY;
      m_spawnQueue.push_back({spawnX, y, delay, ""});
    }
  }

  /**
   * @brief Spawn a single Enemy Red
   */
  static void spawnEnemyRed(ecs::World &world, float posX, float posY)
  {
    ecs::Entity enemy = world.createEntity();

    // Each enemy has its own oscillation pattern
    world.addComponent(enemy, ecs::Pattern{"sine_wave", ENEMY_RED_AMPLITUDE, ENEMY_RED_FREQUENCY});

    ecs::Transform transform;
    transform.x = posX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = ENEMY_RED_SCALE;
    world.addComponent(enemy, transform);

    ecs::Velocity velocity;
    velocity.dx = ENEMY_VELOCITY_X;
    velocity.dy = 0.0F;
    world.addComponent(enemy, velocity);

    ecs::Health health;
    health.hp = ENEMY_HEALTH;
    health.maxHp = ENEMY_HEALTH;
    world.addComponent(enemy, health);

    // Collider scaled to match visual size: frame dimensions * scale
    // Frame: 33x36, Scale: 3.0 => Collider: 99x108
    world.addComponent(
      enemy, ecs::Collider{ENEMY_RED_FRAME_WIDTH * ENEMY_RED_SCALE, ENEMY_RED_SPRITE_HEIGHT * ENEMY_RED_SCALE});

    // Sprite for Enemy Red with animation
    // Spritesheet: 533x36 with 16 frames
    // Frame size: 533/16 = 33px per frame (integer division)
    // Animation: frames 7 to 0 (reverse, from 8th frame to 1st)
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::ENEMY_SHIP;
    sprite.width = ENEMY_RED_FRAME_WIDTH; // Frame width from integer division
    sprite.height = ENEMY_RED_SPRITE_HEIGHT; // Frame height (scale applied via Transform)
    sprite.animated = true;
    sprite.frameCount = ENEMY_RED_FRAME_COUNT;
    sprite.startFrame = 7; // 8th frame (0-indexed)
    sprite.endFrame = 0; // 1st frame
    sprite.currentFrame = 7;
    sprite.frameTime = 0.1f; // 10 FPS animation
    sprite.reverseAnimation = true; // Play from frame 7 to 0
    world.addComponent(enemy, sprite);

    ecs::Networked net;
    net.networkId = enemy;
    world.addComponent(enemy, net);
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
      std::cerr << "[SpawnSystem] No enemy config manager set, falling back to hardcoded enemy" << std::endl;
      spawnEnemyRed(world, posX, posY);
      return;
    }

    const EnemyConfig *config = m_enemyConfigManager->getConfig(enemyType);
    if (!config) {
      std::cerr << "[SpawnSystem] Unknown enemy type: " << enemyType << ", falling back to enemy_red" << std::endl;
      config = m_enemyConfigManager->getConfig("enemy_red");
      if (!config) {
        spawnEnemyRed(world, posX, posY);
        return;
      }
    }

    ecs::Entity enemy = world.createEntity();

    // Pattern component
    world.addComponent(enemy,
                       ecs::Pattern{config->pattern.type, config->pattern.amplitude, config->pattern.frequency});

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

    // Networked component
    ecs::Networked net;
    net.networkId = enemy;
    world.addComponent(enemy, net);
  }

  static void handleSpawnEvent(ecs::World &world, const ecs::SpawnEntityEvent &event)
  {
    switch (event.type) {
    case ecs::SpawnEntityEvent::EntityType::ENEMY:
      spawnEnemyRed(world, event.x, event.y);
      break;
    case ecs::SpawnEntityEvent::EntityType::PROJECTILE:
      spawnProjectile(world, event.x, event.y, event.spawner);
      break;
    case ecs::SpawnEntityEvent::EntityType::POWERUP:
      spawnPowerup(world, event.x, event.y);
      break;
    case ecs::SpawnEntityEvent::EntityType::EXPLOSION:
      spawnExplosion(world, event.x, event.y);
      break;
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

  static void spawnPowerup(ecs::World &world, float posX, float posY)
  {
    (void)world;
    (void)posX;
    (void)posY;
    // TODO: Implement powerup spawning
  }

  static void spawnExplosion(ecs::World &world, float posX, float posY)
  {
    (void)world;
    (void)posX;
    (void)posY;
    // TODO: Implement explosion effect
  }
};

} // namespace server

#endif // SERVER_SPAWN_SYSTEM_HPP_
