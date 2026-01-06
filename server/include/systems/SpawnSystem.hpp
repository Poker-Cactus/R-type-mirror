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
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/Viewport.hpp"
#include "../../../engineCore/include/ecs/components/roles/EnemyAI.hpp"
#include "../../../engineCore/include/ecs/components/roles/Projectile.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <cmath>
#include <random>
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

  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    m_spawnTimer += deltaTime;

    // Spawn enemies periodically
    if (m_spawnTimer >= SPAWN_INTERVAL) {
      spawnRandomEnemy(world);
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

private:
  ecs::EventListenerHandle m_spawnHandle;
  std::mt19937 m_rng;
  float m_spawnTimer = 0.0F;

  // Spawn configuration constants
  static constexpr float SPAWN_INTERVAL = 3.0F;
  static constexpr float DEFAULT_VIEWPORT_WIDTH = 800.0F;
  static constexpr float DEFAULT_VIEWPORT_HEIGHT = 600.0F;
  static constexpr float SPAWN_Y_MARGIN = 50.0F;
  static constexpr float SPAWN_X_OFFSET = 32.0F;
  static constexpr float ENEMY_VELOCITY_X = -150.0F;
  static constexpr int ENEMY_HEALTH = 30;
  static constexpr float ENEMY_COLLIDER_SIZE = 32.0F;
  static constexpr unsigned int ENEMY_SPRITE_SIZE = 64;
  static constexpr float PROJECTILE_COLLIDER_SIZE = 8.0F;
  static constexpr unsigned int PROJECTILE_SPRITE_WIDTH = 84;
  static constexpr unsigned int PROJECTILE_SPRITE_HEIGHT = 36;
  static constexpr float PROJECTILE_VELOCITY_MULTIPLIER = 1700.0F;
  static constexpr float DIRECTION_THRESHOLD = 0.001F;

  void spawnRandomEnemy(ecs::World &world)
  {
    // Get viewport dimensions from connected players
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

    // Random Y position across full screen height
    std::uniform_real_distribution<float> yDist(SPAWN_Y_MARGIN, worldHeight - SPAWN_Y_MARGIN);

    // Spawn at right edge of screen (at the visible border)
    ecs::SpawnEntityEvent event(ecs::SpawnEntityEvent::EntityType::ENEMY,
                                worldWidth - SPAWN_X_OFFSET, // Spawn at right edge (minus half sprite width)
                                yDist(m_rng), 0);
    world.emitEvent(event);
  }
  static void handleSpawnEvent(ecs::World &world, const ecs::SpawnEntityEvent &event)
  {
    switch (event.type) {
    case ecs::SpawnEntityEvent::EntityType::ENEMY:
      spawnEnemy(world, event.x, event.y);
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

  static void spawnEnemy(ecs::World &world, float posX, float posY)
  {
    ecs::Entity enemy = world.createEntity();

    world.addComponent(enemy, ecs::EnemyAI{});

    ecs::Transform transform;
    transform.x = posX;
    transform.y = posY;
    transform.rotation = 0.0F;
    transform.scale = 1.0F;
    world.addComponent(enemy, transform);

    ecs::Velocity velocity;
    velocity.dx = ENEMY_VELOCITY_X;
    velocity.dy = 0.0F;
    world.addComponent(enemy, velocity);

    ecs::Health health;
    health.hp = ENEMY_HEALTH;
    health.maxHp = ENEMY_HEALTH;
    world.addComponent(enemy, health);

    world.addComponent(enemy, ecs::Collider{ENEMY_COLLIDER_SIZE, ENEMY_COLLIDER_SIZE});

    // SERVER ASSIGNS VISUAL IDENTITY AS DATA
    // The sprite is assigned at creation time - never inferred by systems
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::ENEMY_SHIP;
    sprite.width = ENEMY_SPRITE_SIZE;
    sprite.height = ENEMY_SPRITE_SIZE;
    world.addComponent(enemy, sprite);

    ecs::Networked net;
    net.networkId = enemy;
    world.addComponent(enemy, net);
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

    world.addComponent(projectile, ecs::Projectile{});

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
