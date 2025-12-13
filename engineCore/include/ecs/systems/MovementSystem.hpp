/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** MovementSystem
*/

#ifndef MOVEMENTSYSTEM_HPP_
#define MOVEMENTSYSTEM_HPP_
#include "../ISystem.hpp"
#include "../World.hpp"
#include "../components/Transform.hpp"
#include "../components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include <vector>

namespace ecs
{
/**
 * @brief System that updates entity positions based on their velocity
 *
 * This system processes all entities with Transform and Velocity components,
 * updating their position each frame based on deltaTime.
 */
class MovementSystem : public ISystem
{
public:
  MovementSystem() = default;
  void update(World &world, float deltaTime) override
  {
    std::vector<Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      auto &transform = world.getComponent<Transform>(entity);
      auto &velocity = world.getComponent<Velocity>(entity);

      transform.x += velocity.dx * deltaTime;
      transform.y += velocity.dy * deltaTime;
    }
  };

  [[nodiscard]] ComponentSignature getSignature() const override
  {
    ComponentSignature sig;
    sig.set(getComponentId<Transform>());
    sig.set(getComponentId<Velocity>());
    return sig;
  }
};
} // namespace ecs

#endif /* !MOVEMENTSYSTEM_HPP_ */
