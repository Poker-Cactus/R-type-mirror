/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** FollowerSystem.hpp - Updates position of entities that follow a parent
*/

#ifndef SERVER_FOLLOWER_SYSTEM_HPP_
#define SERVER_FOLLOWER_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

/**
 * @brief System that updates the position of follower entities
 *
 * Entities with a Follower component will smoothly move towards
 * their target position (parent position + offset).
 */
class FollowerSystem : public ecs::ISystem
{
public:
  FollowerSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (const auto &entity : entities) {
      if (!world.isAlive(entity)) {
        continue;
      }

      auto &follower = world.getComponent<ecs::Follower>(entity);
      auto &transform = world.getComponent<ecs::Transform>(entity);

      // Check if parent is still alive
      if (!world.isAlive(follower.parent)) {
        // Parent is dead, destroy the follower
        world.destroyEntity(entity);
        continue;
      }

      // Get parent's transform
      if (!world.hasComponent<ecs::Transform>(follower.parent)) {
        continue;
      }

      const auto &parentTransform = world.getComponent<ecs::Transform>(follower.parent);

      // Calculate target position (parent position + offset)
      float targetX = parentTransform.x + follower.offsetX;
      float targetY = parentTransform.y + follower.offsetY;

      // Smoothly interpolate towards target position
      float lerpFactor = 1.0f - std::exp(-follower.smoothing * deltaTime);
      transform.x += (targetX - transform.x) * lerpFactor;
      transform.y += (targetY - transform.y) * lerpFactor;
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Follower>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }
};

} // namespace server

#endif // SERVER_FOLLOWER_SYSTEM_HPP_
