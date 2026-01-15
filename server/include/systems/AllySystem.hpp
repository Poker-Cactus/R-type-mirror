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
    m_oscillationTime += deltaTime * OSCILLATION_SPEED;

    // Check if solo mode (only one player)
    std::vector<ecs::Entity> players;
    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    world.getEntitiesWithSignature(playerSig, players);
    bool isSoloMode = (players.size() == 1);

    std::vector<ecs::Entity> allyEntities;
    world.getEntitiesWithSignature(getSignature(), allyEntities);

    for (auto allyEntity : allyEntities) {
      // Get ally entity's position
      auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
      auto &allyVelocity = world.getComponent<ecs::Velocity>(allyEntity);

      // Find player position
      if (players.empty()) {
        continue; // No player found
      }
      ecs::Entity playerEntity = players[0];
      auto &playerTransform = world.getComponent<ecs::Transform>(playerEntity);

      if (isSoloMode) {
        // In solo mode, move up at 50 pixels per second (100 pixels every 2 seconds)
        allyVelocity.dx = 0.0f;
        allyVelocity.dy = -50.0f;
        // Update animation to up
        auto &sprite = world.getComponent<ecs::Sprite>(allyEntity);
        sprite.currentFrame = 4;
        continue;
      }

      // Find nearest enemy
      ecs::Entity nearestEnemy = findNearestEnemy(world, allyTransform.x, allyTransform.y);
      if (nearestEnemy != 0) {
        auto &enemyTransform = world.getComponent<ecs::Transform>(nearestEnemy);

        // Ally logic here (to be implemented)
        // For now, just move towards player and shoot at enemies
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, enemyTransform, deltaTime);
        updateAllyShooting(world, allyEntity, allyTransform, enemyTransform);
      } else {
        // No enemies, follow player
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, playerTransform, deltaTime);
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
  // Oscillation parameters
  float m_oscillationTime = 0.0f;
  static constexpr float OSCILLATION_SPEED = 2.0f;
  static constexpr float OSCILLATION_AMPLITUDE = 50.0f;
  static constexpr float ALLY_SPEED = 200.0f;

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

  void updateAllyMovement(ecs::World &world, ecs::Entity allyEntity, ecs::Velocity &velocity, const ecs::Transform &allyTransform,
                          const ecs::Transform &targetTransform, float deltaTime)
  {
    // Simple autonomous movement: follow player with some oscillation

    float dx = targetTransform.x - allyTransform.x;
    float dy = targetTransform.y - allyTransform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0) {
      // Move towards target
      velocity.dx = (dx / distance) * ALLY_SPEED;
      velocity.dy = (dy / distance) * ALLY_SPEED;

      // Add lateral oscillation perpendicular to movement direction
      float perpendicularX = -dy / distance; // Perpendicular vector
      float perpendicularY = dx / distance;
      float oscillation = std::sin(m_oscillationTime) * OSCILLATION_AMPLITUDE;

      velocity.dx += perpendicularX * oscillation * deltaTime;
      velocity.dy += perpendicularY * oscillation * deltaTime;
      
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
      // No target, just oscillate in place
      velocity.dx = std::sin(m_oscillationTime) * OSCILLATION_AMPLITUDE * deltaTime;
      velocity.dy = std::cos(m_oscillationTime) * OSCILLATION_AMPLITUDE * deltaTime;

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
