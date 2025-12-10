/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LifetimeSystem.hpp - Handles off-screen entity cleanup
*/

#ifndef SERVER_LIFETIME_SYSTEM_HPP_
#define SERVER_LIFETIME_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

/**
 * @brief System that destroys entities that go off-screen
 */
class LifetimeSystem : public ecs::ISystem
{
public:
  LifetimeSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    std::vector<ecs::Entity> toDestroy;

    for (auto entity : entities) {
      const auto &transform = world.getComponent<ecs::Transform>(entity);

      // Destroy if off-screen (left, right, top, or bottom)
      if (transform.x < -100.0F || transform.x > 900.0F || transform.y < -100.0F || transform.y > 700.0F) {
        toDestroy.push_back(entity);
      }
    }

    for (auto entity : toDestroy) {
      world.destroyEntity(entity);
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }
};

} // namespace server

#endif // SERVER_LIFETIME_SYSTEM_HPP_
