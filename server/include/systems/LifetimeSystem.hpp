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
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Viewport.hpp"
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
      // Players are authoritative and must not disappear; keep them in bounds.
      if (world.hasComponent<ecs::PlayerId>(entity)) {
        auto &t = world.getComponent<ecs::Transform>(entity);
        float playerW = 0.0F;
        float playerH = 0.0F;
        if (world.hasComponent<ecs::Collider>(entity)) {
          const auto &col = world.getComponent<ecs::Collider>(entity);
          if (col.shape == ecs::Collider::Shape::BOX) {
            if (col.width > 0.0F)
              playerW = col.width;
            if (col.height > 0.0F)
              playerH = col.height;
          } else {
            // Circle: clamp using diameter so the whole collider stays visible.
            const float diameter = col.radius * 2.0F;
            if (diameter > 0.0F) {
              playerW = diameter;
              playerH = diameter;
            }
          }
        }

        float maxX = 0.0F;
        float maxY = 0.0F;
        if (world.hasComponent<ecs::Viewport>(entity)) {
          const auto &vp = world.getComponent<ecs::Viewport>(entity);
          if (vp.width > 0) {
            maxX = static_cast<float>(vp.width) - playerW;
          }
          if (vp.height > 0) {
            maxY = static_cast<float>(vp.height) - playerH;
          }
          if (maxX < 0.0F)
            maxX = 0.0F;
          if (maxY < 0.0F)
            maxY = 0.0F;
        } else {
          // Fallback defaults if no viewport info received yet.
          maxX = 800.0F - playerW;
          maxY = 600.0F - playerH;
        }

        if (t.x < 0.0F)
          t.x = 0.0F;
        if (t.x > maxX)
          t.x = maxX;
        if (t.y < 0.0F)
          t.y = 0.0F;
        if (t.y > maxY)
          t.y = maxY;
        continue;
      }
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
