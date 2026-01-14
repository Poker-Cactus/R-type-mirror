/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** InputMovementSystem - applies Input to Velocity (server authoritative)
*/

#ifndef INPUTMOVEMENTSYSTEM_HPP_
#define INPUTMOVEMENTSYSTEM_HPP_

#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/ShipStats.hpp"
#include "../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include <vector>

namespace server
{
class InputMovementSystem : public ecs::ISystem
{
public:
  InputMovementSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      auto &input = world.getComponent<ecs::Input>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);

      // Get ship's move speed from ShipStats (default to 200.0f if not present)
      float speed = 200.0f;
      if (world.hasComponent<ecs::ShipStats>(entity)) {
        speed = world.getComponent<ecs::ShipStats>(entity).moveSpeed;
      }

      velocity.dx = 0.0F;
      velocity.dy = 0.0F;

      if (input.left) {
        velocity.dx -= speed;
      }
      if (input.right) {
        velocity.dx += speed;
      }
      if (input.up) {
        velocity.dy -= speed;
      }
      if (input.down) {
        velocity.dy += speed;
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Input>());
    sig.set(ecs::getComponentId<ecs::Velocity>());
    return sig;
  }
};
} // namespace server

#endif /* !INPUTMOVEMENTSYSTEM_HPP_ */
