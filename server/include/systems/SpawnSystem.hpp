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
#include "../../../engineCore/include/ecs/components/Lifetime.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/roles/EnemyAI.hpp"
#include "../../../engineCore/include/ecs/components/roles/Projectile.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <random>

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
  const float SPAWN_INTERVAL = 3.0F;

  void spawnRandomEnemy(ecs::World &world)
  {
    std::uniform_real_distribution<float> yDist(100.0F, 500.0F);

    ecs::SpawnEntityEvent event(ecs::SpawnEntityEvent::EntityType::ENEMY,
                                1200.0F, // Farther right so enemies enter from off-screen
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

  static void spawnEnemy(ecs::World &world, float x, float y)
  {
    ecs::Entity enemy = world.createEntity();

    world.addComponent(enemy, ecs::EnemyAI{});

    ecs::Transform transform;
    transform.x = x;
    transform.y = y;
    transform.rotation = 0.0F;
    transform.scale = 1.0F;
    world.addComponent(enemy, transform);

    ecs::Velocity velocity;
    velocity.dx = -50.0F;
    velocity.dy = 0.0F;
    world.addComponent(enemy, velocity);

    ecs::Health health;
    health.hp = 30;
    health.maxHp = 30;
    world.addComponent(enemy, health);

    world.addComponent(enemy, ecs::Collider{32.0F, 32.0F});

    // SERVER ASSIGNS VISUAL IDENTITY AS DATA
    // The sprite is assigned at creation time - never inferred by systems
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::ENEMY_SHIP;
    sprite.width = 32;
    sprite.height = 32;
    world.addComponent(enemy, sprite);

    ecs::Networked net;
    net.networkId = enemy;
    world.addComponent(enemy, net);
  }

  static void spawnProjectile(ecs::World &world, float x, float y, ecs::Entity owner)
  {
    // Determine direction: prefer requested direction, fall back to owner velocity
    float directionX = 0.0F;
    if (std::abs(directionX) < 0.001F && world.hasComponent<ecs::Velocity>(owner)) {
      directionX = world.getComponent<ecs::Velocity>(owner).dx;
    }
    if (std::abs(directionX) < 0.001F) {
      directionX = 1.0F; // Default to right if nothing else is known
    }

    const float normalizedDirX = directionX >= 0.0F ? 1.0F : -1.0F;
    const float projectileVelocity = 850.0F * normalizedDirX; // Match player projectile speed

    ecs::Entity projectile = world.createEntity();

    world.addComponent(projectile, ecs::Projectile{});

    // Capability-based offset: use GunOffset if entity has it (no identity checks).
    // Systems ask "What can this entity do?" not "What kind is it?"
    float offsetX = 0.0F;
    if (world.hasComponent<ecs::GunOffset>(owner)) {
      offsetX = world.getComponent<ecs::GunOffset>(owner).x * normalizedDirX;
    }

    ecs::Transform transform;
    transform.x = x + offsetX;
    transform.y = y;
    transform.rotation = 0.0F;
    transform.scale = 1.0F;
    world.addComponent(projectile, transform);

    ecs::Velocity velocity;
    velocity.dx = projectileVelocity;
    velocity.dy = 0.0F;
    world.addComponent(projectile, velocity);

    // Despawn is handled by LifetimeSystem when projectile leaves the viewport.

    world.addComponent(projectile, ecs::Collider{8.0F, 8.0F});

    // SERVER ASSIGNS VISUAL IDENTITY AS DATA
    // Projectile sprite decided at creation, never inferred later
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::PROJECTILE;
    sprite.width = 42;  // 211x92 aspect ratio (422/2 frames, scaled down ~5x)
    sprite.height = 18;
    world.addComponent(projectile, sprite);

    // Mark as networked so the snapshot system replicates it to clients.
    ecs::Networked net;
    net.networkId = projectile;
    world.addComponent(projectile, net);
  }

  static void spawnPowerup(ecs::World &world, float x, float y)
  {
    (void)world;
    (void)x;
    (void)y;
    // TODO: Implement powerup spawning
  }

  static void spawnExplosion(ecs::World &world, float x, float y)
  {
    (void)world;
    (void)x;
    (void)y;
    // TODO: Implement explosion effect
  }
};

} // namespace server

#endif // SERVER_SPAWN_SYSTEM_HPP_
