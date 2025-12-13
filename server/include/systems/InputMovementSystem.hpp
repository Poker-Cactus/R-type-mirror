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

    constexpr float SPEED = 200.0F;

    for (auto entity : entities) {
      auto &input = world.getComponent<ecs::Input>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);

      velocity.dx = 0.0F;
      velocity.dy = 0.0F;

      if (input.left) {
        velocity.dx -= SPEED;
      }
      if (input.right) {
        velocity.dx += SPEED;
      }
      if (input.up) {
        velocity.dy -= SPEED;
      }
      if (input.down) {
        velocity.dy += SPEED;
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
