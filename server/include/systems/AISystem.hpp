/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AISystem.hpp - AI system for controlling AI entities
*/

#ifndef SERVER_AI_SYSTEM_HPP_
#define SERVER_AI_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/AI.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

/**
 * @brief System that controls AI entities
 *
 * This system handles the behavior of AI-controlled entities, such as
 * movement decisions, shooting, and other actions based on game state.
 */
class AISystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> aiEntities;
    world.getEntitiesWithSignature(getSignature(), aiEntities);

    for (auto aiEntity : aiEntities) {
      // Get AI entity's position
      auto &aiTransform = world.getComponent<ecs::Transform>(aiEntity);
      auto &aiVelocity = world.getComponent<ecs::Velocity>(aiEntity);

      // Find player position
      ecs::Entity playerEntity = findPlayer(world);
      if (playerEntity == 0) {
        continue; // No player found
      }
      auto &playerTransform = world.getComponent<ecs::Transform>(playerEntity);

      // Find nearest enemy
      ecs::Entity nearestEnemy = findNearestEnemy(world, aiTransform.x, aiTransform.y);
      if (nearestEnemy != 0) {
        auto &enemyTransform = world.getComponent<ecs::Transform>(nearestEnemy);

        // AI logic here (to be implemented)
        // For now, just move towards player and shoot at enemies
        updateAIMovement(world, aiEntity, aiVelocity, aiTransform, playerTransform, enemyTransform, deltaTime);
        updateAIShooting(world, aiEntity, aiTransform, enemyTransform);
      } else {
        // No enemies, follow player
        updateAIMovement(world, aiEntity, aiVelocity, aiTransform, playerTransform, playerTransform, deltaTime);
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::AI>());
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
   * @brief Update AI movement (placeholder)
   */
  void updateAIMovement(ecs::World &world, ecs::Entity aiEntity, ecs::Velocity &velocity, const ecs::Transform &aiTransform,
                        const ecs::Transform &playerTransform, const ecs::Transform &targetTransform,
                        float deltaTime)
  {
    // Placeholder: move towards target
    // AI logic to be implemented here
    constexpr float AI_SPEED = 200.0f;

    float dx = targetTransform.x - aiTransform.x;
    float dy = targetTransform.y - aiTransform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0) {
      velocity.dx = (dx / distance) * AI_SPEED;
      velocity.dy = (dy / distance) * AI_SPEED;
      
      // Update animation based on movement direction
      auto &sprite = world.getComponent<ecs::Sprite>(aiEntity);
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
      auto &sprite = world.getComponent<ecs::Sprite>(aiEntity);
      sprite.currentFrame = 2; // Neutral frame
    }
  }

  /**
   * @brief Update AI shooting (placeholder)
   */
  void updateAIShooting(ecs::World &world, ecs::Entity aiEntity, const ecs::Transform &aiTransform,
                        const ecs::Transform &enemyTransform)
  {
    // Placeholder: shoot at enemy if close enough
    // AI logic to be implemented here
    float dx = enemyTransform.x - aiTransform.x;
    float dy = enemyTransform.y - aiTransform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    constexpr float SHOOT_RANGE = 300.0f;

    if (distance <= SHOOT_RANGE) {
      // Emit shoot event
      ecs::ShootEvent shootEvent(aiEntity, dx / distance, dy / distance);
      world.emitEvent(shootEvent);
    }
  }
};

} // namespace server

#endif // SERVER_AI_SYSTEM_HPP_
