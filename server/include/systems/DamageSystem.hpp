/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** DamageSystem.hpp - Handles damage application from collisions
*/

#ifndef SERVER_DAMAGE_SYSTEM_HPP_
#define SERVER_DAMAGE_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "../../../engineCore/include/ecs/components/Immortal.hpp"
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
      [&world](const ecs::CollisionEvent &event) { handleCollision(world, event); });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    // Empty signature - we work via events
    return {};
  }

private:
  ecs::EventListenerHandle m_collisionHandle;
  static constexpr int damageFromProjectile = 20;
  static constexpr int damageFromEntityCollision = 10;

  static void handleCollision(ecs::World &world, const ecs::CollisionEvent &event)
  {
    ecs::Entity entityA = event.entityA;
    ecs::Entity entityB = event.entityB;

    // Check if entities are still alive (might have been destroyed in a previous collision)
    if (!world.isAlive(entityA) || !world.isAlive(entityB)) {
      return;
    }

    // Check if either entity is a projectile owned by the other (prevent self-damage)
    if (world.hasComponent<ecs::Owner>(entityA)) {
      const auto &ownerA = world.getComponent<ecs::Owner>(entityA);
      if (ownerA.ownerId == entityB) {
        return; // A is owned by B, ignore collision
      }
    }
    if (world.hasComponent<ecs::Owner>(entityB)) {
      const auto &ownerB = world.getComponent<ecs::Owner>(entityB);
      if (ownerB.ownerId == entityA) {
        return; // B is owned by A, ignore collision
      }
    }

    // Apply damage if both entities have health (entity-on-entity collision)
    bool aHasHealth = world.hasComponent<ecs::Health>(entityA);
    bool bHasHealth = world.hasComponent<ecs::Health>(entityB);

    if (aHasHealth && bHasHealth) {
      // Both have health - if both are players, ignore collision damage
      bool aIsPlayer = world.hasComponent<ecs::Input>(entityA);
      bool bIsPlayer = world.hasComponent<ecs::Input>(entityB);

      if (aIsPlayer && bIsPlayer) {
        // Do not apply damage when two players collide to avoid instant kills
        return;
      }

      // Otherwise apply mutual damage (e.g., enemy vs player)
      applyDamage(world, entityA, entityB, damageFromEntityCollision);
      applyDamage(world, entityB, entityA, damageFromEntityCollision);
    } else if (aHasHealth && !bHasHealth) {
      // Only A has health - projectile B hitting entity A
      applyDamage(world, entityA, entityB, damageFromProjectile);
      
      // Check if projectile B is immortal before destroying
      if (!world.hasComponent<ecs::Immortal>(entityB) || 
          !world.getComponent<ecs::Immortal>(entityB).isImmortal) {
        world.destroyEntity(entityB); // Destroy projectile only if not immortal
      }
    } else if (!aHasHealth && bHasHealth) {
      // Only B has health - projectile A hitting entity B
      applyDamage(world, entityB, entityA, damageFromProjectile);
      
      // Check if projectile A is immortal before destroying
      if (!world.hasComponent<ecs::Immortal>(entityA) || 
          !world.getComponent<ecs::Immortal>(entityA).isImmortal) {
        world.destroyEntity(entityA); // Destroy projectile only if not immortal
      }
    }
  }

  static void applyDamage(ecs::World &world, ecs::Entity target, ecs::Entity source, int damage)
  {
    if (!world.isAlive(target) || !world.hasComponent<ecs::Health>(target)) {
      return;
    }

    // Find the real source (if source is a projectile, get its owner)
    ecs::Entity realSource = source;
    if (world.hasComponent<ecs::Owner>(source)) {
      const auto &owner = world.getComponent<ecs::Owner>(source);
      if (world.isAlive(owner.ownerId)) {
        realSource = owner.ownerId; // Credit the owner, not the projectile
      }
    }

    // Prevent friendly fire: if source is a player and target is also a player, skip
    if (realSource != 0 && world.hasComponent<ecs::Input>(realSource) && world.hasComponent<ecs::Input>(target)) {
      return;
    }

    if (world.hasComponent<ecs::Immortal>(target)) {
      const auto &immortal = world.getComponent<ecs::Immortal>(target);
      if (immortal.isImmortal) {
        return; // Do not apply damage to immortal entities
      }
    }

    // Emit damage event
    ecs::DamageEvent damageEvent(target, realSource, damage);
    world.emitEvent(damageEvent);

    auto &health = world.getComponent<ecs::Health>(target);
    health.hp -= damage;

    if (health.hp <= 0) {
      health.hp = 0;
      // Emit death event with the real killer
      ecs::DeathEvent deathEvent(target, realSource);
      world.emitEvent(deathEvent);
    }
  }
};

} // namespace server

#endif // SERVER_DAMAGE_SYSTEM_HPP_
