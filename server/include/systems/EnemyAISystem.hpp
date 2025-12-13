/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EnemyAISystem.hpp - Enemy AI behavior
*/

#ifndef SERVER_ENEMY_AI_SYSTEM_HPP_
#define SERVER_ENEMY_AI_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <random>
#include <unordered_map>
#include <vector>

namespace server
{

/**
 * @brief System that controls enemy AI behavior
 */
class EnemyAISystem : public ecs::ISystem
{
public:
  EnemyAISystem() : m_rng(std::random_device{}()) {}

  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      // Only run AI on enemies; never on player-controlled entities.
      if (world.hasComponent<ecs::PlayerId>(entity)) {
        continue;
      }
      auto &transform = world.getComponent<ecs::Transform>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);

      m_aiTimer[entity] += deltaTime;

      // Simple AI: enemies with negative velocity move left
      if (velocity.dx < 0) {
        // Keep moving left
        velocity.dx = -80.0F;
      }

      // Shoot periodically (every 2 seconds)
      if (m_aiTimer[entity] >= 2.0F) {
        ecs::ShootEvent shootEvent(entity, -1.0F, 0.0F);
        world.emitEvent(shootEvent);
        m_aiTimer[entity] = 0.0F;
      }

      // Destroy if off-screen (left side)
      if (transform.x < -100.0F) {
        world.destroyEntity(entity);
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Velocity>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }

private:
  std::mt19937 m_rng;
  std::unordered_map<ecs::Entity, float> m_aiTimer;
};

} // namespace server

#endif // SERVER_ENEMY_AI_SYSTEM_HPP_
