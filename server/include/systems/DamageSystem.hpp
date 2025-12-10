/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** DamageSystem.hpp - Handles damage application from collisions
*/

#ifndef SERVER_DAMAGE_SYSTEM_HPP_
#define SERVER_DAMAGE_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "ecs/ComponentSignature.hpp"

namespace server
{

/**
 * @brief System that listens to CollisionEvent and applies damage
 */
class DamageSystem : public ecs::ISystem
{
public:
  DamageSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;
    (void)world;
    // This system works via event subscription, see initialize()
  }

  /**
   * @brief Subscribe to collision events
   */
  void initialize(ecs::World &world)
  {
    // Subscribe to collision events
    m_collisionHandle = world.subscribeEvent<ecs::CollisionEvent>(
      [&world](const ecs::CollisionEvent &event) {
        handleCollision(world, event);
      });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    // Empty signature - we work via events
    return {};
  }

private:
  ecs::EventListenerHandle m_collisionHandle;

  static void handleCollision(ecs::World &world, const ecs::CollisionEvent &event)
  {
    ecs::Entity entityA = event.entityA;
    ecs::Entity entityB = event.entityB;

    // Apply damage if both entities have health (entity-on-entity collision)
    bool aHasHealth = world.hasComponent<ecs::Health>(entityA);
    bool bHasHealth = world.hasComponent<ecs::Health>(entityB);

    if (aHasHealth && bHasHealth) {
      // Both have health - mutual damage
      applyDamage(world, entityA, entityB, 10);
      applyDamage(world, entityB, entityA, 10);
    } else if (aHasHealth && !bHasHealth) {
      // Only A has health - projectile B hitting entity A
      applyDamage(world, entityA, entityB, 20);
      world.destroyEntity(entityB); // Destroy projectile
    } else if (!aHasHealth && bHasHealth) {
      // Only B has health - projectile A hitting entity B
      applyDamage(world, entityB, entityA, 20);
      world.destroyEntity(entityA); // Destroy projectile
    }
  }

  static void applyDamage(ecs::World &world, ecs::Entity target, ecs::Entity source, 
                         int damage)
  {
    if (!world.hasComponent<ecs::Health>(target)) {
      return;
    }

    // Emit damage event
    ecs::DamageEvent damageEvent(target, source, damage);
    world.emitEvent(damageEvent);

    auto &health = world.getComponent<ecs::Health>(target);
    health.hp -= damage;

    if (health.hp <= 0) {
      health.hp = 0;
      // Emit death event
      ecs::DeathEvent deathEvent(target, source);
      world.emitEvent(deathEvent);
    }
  }
};

} // namespace server

#endif // SERVER_DAMAGE_SYSTEM_HPP_
