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
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/roles/EnemyAI.hpp"
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
      auto &transform = world.getComponent<ecs::Transform>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);

      m_aiTimer[entity] += deltaTime;

      // Initialize per-entity randomized shot delay
      if (!m_nextShotDelay.contains(entity)) {
        m_nextShotDelay[entity] = randomShotDelay();
      }

      // Simple AI: enemies with negative velocity move left
      if (velocity.dx < 0) {
        // Keep moving left
        velocity.dx = ENEMY_MOVE_SPEED;
      }

      // Shoot after a random cooldown so enemies feel less predictable
      if (m_aiTimer[entity] >= m_nextShotDelay[entity]) {
        ecs::ShootEvent shootEvent(entity, -1.0F, 0.0F);
        world.emitEvent(shootEvent);
        m_aiTimer[entity] = 0.0F;
        m_nextShotDelay[entity] = randomShotDelay();
      }

      // Destroy if off-screen (left side)
      if (transform.x < OFFSCREEN_DESTROY_X) {
        world.destroyEntity(entity);
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::EnemyAI>());
    sig.set(ecs::getComponentId<ecs::Velocity>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }

private:
  std::mt19937 m_rng;
  std::unordered_map<ecs::Entity, float> m_aiTimer;
  std::unordered_map<ecs::Entity, float> m_nextShotDelay;

  // AI behavior constants
  static constexpr float ENEMY_MOVE_SPEED = -80.0F;
  static constexpr float MIN_SHOT_DELAY = 1.5F;
  static constexpr float MAX_SHOT_DELAY = 3.5F;
  static constexpr float OFFSCREEN_DESTROY_X = -100.0F;

  float randomShotDelay()
  {
    std::uniform_real_distribution<float> dist(MIN_SHOT_DELAY, MAX_SHOT_DELAY);
    return dist(m_rng);
  }
};

} // namespace server

#endif // SERVER_ENEMY_AI_SYSTEM_HPP_
