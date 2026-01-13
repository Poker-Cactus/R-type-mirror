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
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <cmath>
#include <random>
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

  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    m_spawnTimer += deltaTime;
    m_powerupSpawnTimer += deltaTime;

    // Update spawn queue timer for delayed spawns
    if (!m_spawnQueue.empty()) {
      m_spawnQueueTimer += deltaTime;

      while (!m_spawnQueue.empty() && m_spawnQueueTimer >= m_spawnQueue.front().delay) {
        const auto &queuedSpawn = m_spawnQueue.front();
        spawnEnemyRed(world, queuedSpawn.x, queuedSpawn.y);
        m_spawnQueueTimer -= queuedSpawn.delay;
        m_spawnQueue.erase(m_spawnQueue.begin());
      }
    }

    // Spawn enemy groups periodically
    if (m_spawnTimer >= SPAWN_INTERVAL) {
      spawnEnemyRedGroup(world);
      m_spawnTimer = 0.0F;
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

private:
  ecs::EventListenerHandle m_spawnHandle;
  std::mt19937 m_rng;
  float m_spawnTimer = 0.0F;
  float m_spawnQueueTimer = 0.0F;
  float m_powerupSpawnTimer = 0.0F;
  int m_powerupSpawnCount = 0; // Counter to alternate powerup types: 0=DRONE, 1=BUBBLE

  struct QueuedSpawn {
    float x;
    float y;
    float delay;
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
  static constexpr float PROJECTILE_VELOCITY_MULTIPLIER = 400.0F;
  static constexpr float DIRECTION_THRESHOLD = 0.01F;

  // Ruban/Wave beam projectile configuration (R-Type ribbon effect)
  // Uses xruban_projectile.png format (x = phase 1-14)
  // Phase 1 initial dimensions: 21x49, 1 frame
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
      powerupType = PowerupType::BUBBLE_RUBAN;

      // powerupType = PowerupType::DRONE;
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
      m_spawnQueue.push_back({spawnX, y, delay});
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

  static void handleSpawnEvent(ecs::World &world, const ecs::SpawnEntityEvent &event)
  {
    configRubanProjectile config;
    switch (event.type) {
    case ecs::SpawnEntityEvent::EntityType::ENEMY:
      spawnEnemyRed(world, event.x, event.y);
      break;
    case ecs::SpawnEntityEvent::EntityType::PROJECTILE:
      spawnProjectile(world, event.x, event.y, event.spawner);
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

    const float projectileVelocity = PROJECTILE_VELOCITY_MULTIPLIER * 1.0F;

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

    ecs::Entity projectile = world.createEntity();

    // Capability-based offset: use GunOffset if entity has it
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

    world.addComponent(projectile, ecs::Collider{PROJECTILE_COLLIDER_SIZE, PROJECTILE_COLLIDER_SIZE});

    // Sprite for triple projectile
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::TRIPLE_PROJECTILE;
    sprite.width = PROJECTILE_SPRITE_WIDTH;
    sprite.height = PROJECTILE_SPRITE_HEIGHT;
    sprite.animated = true;
    sprite.frameCount = 3;
    sprite.loop = false;
    sprite.startFrame = 0;
    sprite.endFrame = 2;

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
    sprite.frameTime = 0.03f; // Animation speed
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
    sprite.frameTime = 0.1f;
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
};

} // namespace server

#endif // SERVER_SPAWN_SYSTEM_HPP_
