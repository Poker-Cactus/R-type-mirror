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
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/components/Immortal.hpp"
#include "../../../engineCore/include/ecs/components/Invulnerable.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/Shield.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
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

    // Skip if either entity is a follower (bubble/drone attached to player)
    // Followers should not cause damage - they only block projectiles and shoot
    // Shields are followers but must take damage.
    bool aIsFollower = world.hasComponent<ecs::Follower>(entityA);
    bool bIsFollower = world.hasComponent<ecs::Follower>(entityB);
    bool aIsShield = world.hasComponent<ecs::Shield>(entityA);
    bool bIsShield = world.hasComponent<ecs::Shield>(entityB);
    if ((aIsFollower && !aIsShield) || (bIsFollower && !bIsShield)) {
      return;
    }

    // Helper to check if sprite is a bubble/powerup (collectible, should not cause damage)
    auto isBubbleOrPowerup = [](std::uint32_t spriteId) {
      return spriteId == ecs::SpriteId::POWERUP || spriteId == ecs::SpriteId::BUBBLE ||
        spriteId == ecs::SpriteId::BUBBLE_TRIPLE || spriteId == ecs::SpriteId::DRONE ||
        (spriteId >= ecs::SpriteId::BUBBLE_RUBAN1 && spriteId <= ecs::SpriteId::BUBBLE_RUBAN3) ||
        (spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 && spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4);
    };

    // Skip if either entity is a bubble or powerup - PowerupSystem handles those
    bool isAPowerup = false;
    bool isBPowerup = false;

    if (world.hasComponent<ecs::Sprite>(entityA)) {
      const auto &spriteA = world.getComponent<ecs::Sprite>(entityA);
      isAPowerup = isBubbleOrPowerup(spriteA.spriteId);
    }
    if (world.hasComponent<ecs::Sprite>(entityB)) {
      const auto &spriteB = world.getComponent<ecs::Sprite>(entityB);
      isBPowerup = isBubbleOrPowerup(spriteB.spriteId);
    }

    if (isAPowerup || isBPowerup) {
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

      // Also prevent damage when player collides with ally
      bool aIsAlly = world.hasComponent<ecs::Ally>(entityA);
      bool bIsAlly = world.hasComponent<ecs::Ally>(entityB);

      if ((aIsPlayer && bIsAlly) || (aIsAlly && bIsPlayer)) {
        // Do not apply damage when player collides with ally
        return;
      }

      // Otherwise apply mutual damage (e.g., enemy vs player)
      applyDamage(world, entityA, entityB, damageFromEntityCollision);
      applyDamage(world, entityB, entityA, damageFromEntityCollision);

      // If collision is between player and enemy, destroy the enemy immediately (player loses one life)
      bool aIsEnemy = world.hasComponent<ecs::Pattern>(entityA) && !aIsPlayer;
      bool bIsEnemy = world.hasComponent<ecs::Pattern>(entityB) && !bIsPlayer;

      if (aIsEnemy && bIsPlayer && world.isAlive(entityA)) {
        world.destroyEntity(entityA);
      }
      if (bIsEnemy && aIsPlayer && world.isAlive(entityB)) {
        world.destroyEntity(entityB);
      }
    } else if (aHasHealth && !bHasHealth) {
      // Only A has health - projectile B hitting entity A
      applyDamage(world, entityA, entityB, damageFromProjectile);

      // Check if projectile B is immortal - if so, don't destroy it
      if (world.hasComponent<ecs::Immortal>(entityB) && world.getComponent<ecs::Immortal>(entityB).isImmortal) {
        return; // Don't destroy immortal projectiles
      }

      // Check if projectile should be destroyed: don't destroy enemy projectiles hitting enemies
      bool shouldDestroyProjectile = true;
      if (world.hasComponent<ecs::Owner>(entityB)) {
        const auto &owner = world.getComponent<ecs::Owner>(entityB);
        if (world.isAlive(owner.ownerId)) {
          bool projectileOwnerIsEnemy = world.hasComponent<ecs::Pattern>(owner.ownerId);
          bool targetIsEnemy = world.hasComponent<ecs::Pattern>(entityA);
          if (projectileOwnerIsEnemy && targetIsEnemy) {
            shouldDestroyProjectile = false; // Enemy projectile passes through enemies
          }
          // Prevent destruction in friendly fire between allies and players
          bool ownerIsAlly = world.hasComponent<ecs::Ally>(owner.ownerId);
          bool targetIsPlayer = world.hasComponent<ecs::Input>(entityA);
          bool ownerIsPlayer = world.hasComponent<ecs::Input>(owner.ownerId);
          bool targetIsAlly = world.hasComponent<ecs::Ally>(entityA);
          if ((ownerIsAlly && targetIsPlayer) || (ownerIsPlayer && targetIsAlly)) {
            shouldDestroyProjectile = false; // Don't destroy in friendly fire
          }
        }
      }
      if (shouldDestroyProjectile) {
        world.destroyEntity(entityB); // Destroy projectile
      }
    } else if (!aHasHealth && bHasHealth) {
      // Only B has health - projectile A hitting entity B
      applyDamage(world, entityB, entityA, damageFromProjectile);

      // Check if projectile A is immortal - if so, don't destroy it
      if (world.hasComponent<ecs::Immortal>(entityA) && world.getComponent<ecs::Immortal>(entityA).isImmortal) {
        return; // Don't destroy immortal projectiles
      }

      // Check if projectile should be destroyed: don't destroy enemy projectiles hitting enemies
      bool shouldDestroyProjectile = true;
      if (world.hasComponent<ecs::Owner>(entityA)) {
        const auto &owner = world.getComponent<ecs::Owner>(entityA);
        if (world.isAlive(owner.ownerId)) {
          bool projectileOwnerIsEnemy = world.hasComponent<ecs::Pattern>(owner.ownerId);
          bool targetIsEnemy = world.hasComponent<ecs::Pattern>(entityB);
          if (projectileOwnerIsEnemy && targetIsEnemy) {
            shouldDestroyProjectile = false; // Enemy projectile passes through enemies
          }
          // Prevent destruction in friendly fire between allies and players
          bool ownerIsAlly = world.hasComponent<ecs::Ally>(owner.ownerId);
          bool targetIsPlayer = world.hasComponent<ecs::Input>(entityB);
          bool ownerIsPlayer = world.hasComponent<ecs::Input>(owner.ownerId);
          bool targetIsAlly = world.hasComponent<ecs::Ally>(entityB);
          if ((ownerIsAlly && targetIsPlayer) || (ownerIsPlayer && targetIsAlly)) {
            shouldDestroyProjectile = false; // Don't destroy in friendly fire
          }
        }
      }
      if (shouldDestroyProjectile) {
        world.destroyEntity(entityA); // Destroy projectile
      }
    }
  }

  static void applyDamage(ecs::World &world, ecs::Entity target, ecs::Entity source, int damage)
  {
    (void)damage; // We use fixed 1 life per hit; keep parameter for compatibility
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

    // Prevent friendly fire between allies and players
    if (realSource != 0 && world.hasComponent<ecs::Ally>(realSource) && world.hasComponent<ecs::Input>(target)) {
      return; // Ally can't damage player
    }
    if (realSource != 0 && world.hasComponent<ecs::Input>(realSource) && world.hasComponent<ecs::Ally>(target)) {
      return; // Player can't damage ally
    }

    // Check immortality first
    if (world.hasComponent<ecs::Immortal>(target)) {
      const auto &immortal = world.getComponent<ecs::Immortal>(target);
      if (immortal.isImmortal) {
        return; // Do not apply damage to immortal entities
      }
    }

    // Prevent enemy friendly fire: if source is an enemy (has Pattern) and target is also an enemy (has Pattern), skip
    bool sourceIsEnemy = realSource != 0 && world.isAlive(realSource) && world.hasComponent<ecs::Pattern>(realSource);
    bool targetIsEnemy = world.hasComponent<ecs::Pattern>(target);

    if (sourceIsEnemy && targetIsEnemy) {
      return;
    }

    // Skip if target is currently invulnerable
    if (world.hasComponent<ecs::Invulnerable>(target)) {
      const auto &inv = world.getComponent<ecs::Invulnerable>(target);
      if (inv.remaining > 0.0F) {
        return; // ignore repeated collision while invulnerable
      }
    }

    // Decide applied damage: players lose exactly 1 life per hit; other entities take full damage value
    int appliedDamage = damage;
    if (world.hasComponent<ecs::Input>(target)) {
      appliedDamage = 1; // players lose one life per hit
    }
    // Emit damage event with applied damage
    ecs::DamageEvent damageEvent(target, realSource, appliedDamage);
    world.emitEvent(damageEvent);

    auto &health = world.getComponent<ecs::Health>(target);
    health.hp -= appliedDamage;

    // If target is a player, give a short invulnerability window to avoid multi-hits while overlapping
    if (world.hasComponent<ecs::Input>(target)) {
      constexpr float INVULNERABILITY_SECONDS = 0.6F;
      if (world.hasComponent<ecs::Invulnerable>(target)) {
        auto &inv = world.getComponent<ecs::Invulnerable>(target);
        inv.remaining = INVULNERABILITY_SECONDS;
      } else {
        world.addComponent<ecs::Invulnerable>(target, ecs::Invulnerable{INVULNERABILITY_SECONDS});
      }
    }

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
