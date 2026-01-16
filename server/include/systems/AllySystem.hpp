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
#include <cmath>
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
    m_shootingTimer += deltaTime;

    // Check if solo mode (only one player)
    std::vector<ecs::Entity> players;
    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    world.getEntitiesWithSignature(playerSig, players);
    bool isSoloMode = (players.size() == 1);

    if (!isSoloMode) {
      return; // Only process allies in solo mode
    }

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

      // Find nearest enemy
      ecs::Entity nearestEnemy = findNearestEnemy(world, allyTransform.x, allyTransform.y);
      if (nearestEnemy != 0) {
        auto &enemyTransform = world.getComponent<ecs::Transform>(nearestEnemy);

        // Ally logic here (to be implemented)
        // For now, just move towards player and shoot at enemies
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, enemyTransform);
        updateAllyShooting(world, allyEntity, allyTransform, enemyTransform);
      } else {
        // No enemies, follow player
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, playerTransform);
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
  static constexpr float ALLY_SPEED = 200.0f;

  // Shooting parameters
  float m_shootingTimer = 0.0f;
  static constexpr float SHOOTING_INTERVAL = 0.5f; // Shoot every 0.5 seconds

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
                          const ecs::Transform &targetTransform)
  {
    float dy = targetTransform.y - allyTransform.y;
    velocity.dx = 0.0f;

    if (std::abs(dy) > 50.0f) {
      velocity.dy = (dy > 0 ? 1.0f : -1.0f) * ALLY_SPEED;
    } else {
      velocity.dy = 0.0f;
    }

    // Update animation based on movement direction
    auto &sprite = world.getComponent<ecs::Sprite>(allyEntity);
    if (velocity.dy < -10.0f) { // Moving up
      sprite.currentFrame = 4; // Up animation frame
    } else if (velocity.dy > 10.0f) { // Moving down
      sprite.currentFrame = 0; // Down animation frame
    } else { // Stopped
      sprite.currentFrame = 2; // Neutral frame
    }
  }

  /**
   * @brief Update ally shooting based on Y alignment with enemy
   */
  void updateAllyShooting(ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
                          const ecs::Transform &enemyTransform)
  {
    float dy = enemyTransform.y - allyTransform.y;

    if (std::abs(dy) <= 50.0f && m_shootingTimer >= SHOOTING_INTERVAL) {
      // Shoot right (assuming enemies are to the right)
      ecs::ShootEvent shootEvent(allyEntity, 1.0f, 0.0f);
      world.emitEvent(shootEvent);
      m_shootingTimer = 0.0f;
    }
  }
};

} // namespace server

#endif // SERVER_ALLY_SYSTEM_HPP_
