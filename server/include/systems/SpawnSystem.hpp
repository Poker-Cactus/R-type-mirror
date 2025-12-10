/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** SpawnSystem.hpp - Handles entity spawning
*/

#ifndef SERVER_SPAWN_SYSTEM_HPP_
#define SERVER_SPAWN_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "ecs/ComponentSignature.hpp"
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
      [&world](const ecs::SpawnEntityEvent &event) {
        handleSpawnEvent(world, event);
      });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    return {};
  }

private:
  ecs::EventListenerHandle m_spawnHandle;
  std::mt19937 m_rng;
  float m_spawnTimer = 0.0F;
  const float SPAWN_INTERVAL = 3.0F;

  void spawnRandomEnemy(ecs::World &world)
  {
    std::uniform_real_distribution<float> yDist(100.0F, 500.0F);
    
    ecs::SpawnEntityEvent event(
      ecs::SpawnEntityEvent::EntityType::ENEMY,
      800.0F, // Right side of screen
      yDist(m_rng),
      0
    );
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
  }

  static void spawnProjectile(ecs::World &world, float x, float y, ecs::Entity owner)
  {
    // Determine direction based on owner's velocity
    float projectileVelocity = 300.0F; // Default to right (player)
    if (world.hasComponent<ecs::Velocity>(owner)) {
      const auto &ownerVel = world.getComponent<ecs::Velocity>(owner);
      // If owner moves left (enemy), projectile goes left
      if (ownerVel.dx < 0) {
        projectileVelocity = -300.0F;
      }
    }
    
    ecs::Entity projectile = world.createEntity();
    
    ecs::Transform transform;
    transform.x = x;
    transform.y = y;
    transform.rotation = 0.0F;
    transform.scale = 1.0F;
    world.addComponent(projectile, transform);
    
    ecs::Velocity velocity;
    velocity.dx = projectileVelocity;
    velocity.dy = 0.0F;
    world.addComponent(projectile, velocity);
    
    world.addComponent(projectile, ecs::Collider{8.0F, 8.0F});
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
