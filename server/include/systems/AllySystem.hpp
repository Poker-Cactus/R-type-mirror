/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllySystem.hpp - Ally system for controlling ally entities
*/

#ifndef SERVER_ALLY_SYSTEM_HPP_
#define SERVER_ALLY_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

/**
 * @brief System that controls ally entities
 *
 * This system handles the behavior of ally-controlled entities, such as
 * movement decisions, shooting, and other actions based on game state.
 */
class AllySystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> allyEntities;
    world.getEntitiesWithSignature(getSignature(), allyEntities);

    for (auto allyEntity : allyEntities) {
      // Get ally entity's position
      auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
      auto &allyVelocity = world.getComponent<ecs::Velocity>(allyEntity);

      // Find player position
      ecs::Entity playerEntity = findPlayer(world);
      if (playerEntity == 0) {
        continue; // No player found
      }
      auto &playerTransform = world.getComponent<ecs::Transform>(playerEntity);

      // Find nearest enemy
      ecs::Entity nearestEnemy = findNearestEnemy(world, allyTransform.x, allyTransform.y);
      if (nearestEnemy != 0) {
        auto &enemyTransform = world.getComponent<ecs::Transform>(nearestEnemy);

        // Ally logic here (to be implemented)
        // For now, just move towards player and shoot at enemies
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, playerTransform, enemyTransform, deltaTime);
        updateAllyShooting(world, allyEntity, allyTransform, enemyTransform);
      } else {
        // No enemies, follow player
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, playerTransform, playerTransform, deltaTime);
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Ally>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    sig.set(ecs::getComponentId<ecs::Velocity>());
    return sig;
  }

private:
  /**
   * @brief Find the player entity
   */
  ecs::Entity findPlayer(ecs::World &world)
  {
    std::vector<ecs::Entity> players;
    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    world.getEntitiesWithSignature(playerSig, players);

    if (!players.empty()) {
      return players[0]; // Assume first player
    }
    return 0;
  }

  /**
   * @brief Find the nearest enemy to the given position
   */
  ecs::Entity findNearestEnemy(ecs::World &world, float x, float y)
  {
    std::vector<ecs::Entity> enemies;
    ecs::ComponentSignature enemySig;
    enemySig.set(ecs::getComponentId<ecs::Transform>());
    enemySig.set(ecs::getComponentId<ecs::Pattern>()); // Enemies have Pattern component
    world.getEntitiesWithSignature(enemySig, enemies);

    ecs::Entity nearest = 0;
    float minDistance = std::numeric_limits<float>::max();

    for (auto enemy : enemies) {
      auto &transform = world.getComponent<ecs::Transform>(enemy);
      float dx = transform.x - x;
      float dy = transform.y - y;
      float distance = std::sqrt(dx * dx + dy * dy);

      if (distance < minDistance) {
        minDistance = distance;
        nearest = enemy;
      }
    }

    return nearest;
  }

  /**
   * @brief Update ally movement (placeholder)
   */
  void updateAllyMovement(ecs::World &world, ecs::Entity allyEntity, ecs::Velocity &velocity, const ecs::Transform &allyTransform,
                          const ecs::Transform &playerTransform, const ecs::Transform &targetTransform,
                          float deltaTime)
  {
    // Placeholder: move towards target
    // Ally logic to be implemented here
    constexpr float ALLY_SPEED = 200.0f;

    float dx = targetTransform.x - allyTransform.x;
    float dy = targetTransform.y - allyTransform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0) {
      velocity.dx = (dx / distance) * ALLY_SPEED;
      velocity.dy = (dy / distance) * ALLY_SPEED;
      
      // Update animation based on movement direction
      auto &sprite = world.getComponent<ecs::Sprite>(allyEntity);
      if (velocity.dy < -50.0f) { // Moving up
        sprite.currentFrame = 4; // Up animation frame
      } else if (velocity.dy > 50.0f) { // Moving down
        sprite.currentFrame = 0; // Down animation frame
      } else { // Moving horizontally or stopped
        sprite.currentFrame = 2; // Neutral frame
      }
    } else {
      velocity.dx = 0;
      velocity.dy = 0;
      // Idle animation
      auto &sprite = world.getComponent<ecs::Sprite>(allyEntity);
      sprite.currentFrame = 2; // Neutral frame
    }
  }

  /**
   * @brief Update ally shooting (placeholder)
   */
  void updateAllyShooting(ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
                          const ecs::Transform &enemyTransform)
  {
    // Placeholder: shoot at enemy if close enough
    // Ally logic to be implemented here
    float dx = enemyTransform.x - allyTransform.x;
    float dy = enemyTransform.y - allyTransform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    constexpr float SHOOT_RANGE = 300.0f;

    if (distance <= SHOOT_RANGE) {
      // Emit shoot event
      ecs::ShootEvent shootEvent(allyEntity, dx / distance, dy / distance);
      world.emitEvent(shootEvent);
    }
  }
};

} // namespace server

#endif // SERVER_ALLY_SYSTEM_HPP_
