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
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
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

    // Compute an authoritative world viewport from connected players.
    // We pick the max width/height to support different client window sizes.
    float worldW = 800.0F;
    float worldH = 600.0F;
    {
      ecs::ComponentSignature playerSig;
      playerSig.set(ecs::getComponentId<ecs::PlayerId>());
      playerSig.set(ecs::getComponentId<ecs::Viewport>());
      std::vector<ecs::Entity> players;
      world.getEntitiesWithSignature(playerSig, players);
      for (auto p : players) {
        const auto &vp = world.getComponent<ecs::Viewport>(p);
        if (vp.width > 0) {
          worldW = std::max(worldW, static_cast<float>(vp.width));
        }
        if (vp.height > 0) {
          worldH = std::max(worldH, static_cast<float>(vp.height));
        }
      }
    }

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

      // Destroy if off-screen (beyond viewport bounds). Treat size via Collider when available.
      float w = 0.0F;
      float h = 0.0F;
      if (world.hasComponent<ecs::Collider>(entity)) {
        const auto &col = world.getComponent<ecs::Collider>(entity);
        if (col.shape == ecs::Collider::Shape::BOX) {
          w = col.width;
          h = col.height;
        } else {
          const float d = col.radius * 2.0F;
          w = d;
          h = d;
        }
      }

      const float left = -w;
      const float right = worldW + w * 2.0F; // Give extra space on the right for enemy spawns
      const float top = -h;
      const float bottom = worldH + h;

      // Only destroy if entity goes off LEFT side or top/bottom (not right side for enemies)
      // This allows enemies to spawn off-screen on the right and come in
      bool offLeft = transform.x < left;
      bool offTopOrBottom = transform.y < top || transform.y > bottom;

      // Only destroy projectiles/enemies that go too far right (prevent infinite accumulation)
      bool tooFarRight = transform.x > right;

      if (offLeft || offTopOrBottom || tooFarRight) {
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
