/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AttractionSystem.hpp
*/

#ifndef SERVER_ATTRACTIONSYSTEM_HPP_
#define SERVER_ATTRACTIONSYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Attraction.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <vector>

namespace server
{
class AttractionSystem : public ecs::ISystem
{
public:
  AttractionSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    // Find all entities with Attraction component
    ecs::ComponentSignature attractionSig;
    attractionSig.set(ecs::getComponentId<ecs::Attraction>());
    attractionSig.set(ecs::getComponentId<ecs::Transform>());

    std::vector<ecs::Entity> attractionEntities;
    world.getEntitiesWithSignature(attractionSig, attractionEntities);

    for (auto attractEntity : attractionEntities) {
      const auto &attraction = world.getComponent<ecs::Attraction>(attractEntity);
      const auto &attractTransform = world.getComponent<ecs::Transform>(attractEntity);

      if (attraction.force <= 0.0F || attraction.radius <= 0.0F) {
        continue; // No attraction to apply
      }

      // Find all entities with Input component (players)
      ecs::ComponentSignature inputSig;
      inputSig.set(ecs::getComponentId<ecs::Input>());
      inputSig.set(ecs::getComponentId<ecs::Transform>());

      std::vector<ecs::Entity> inputEntities;
      world.getEntitiesWithSignature(inputSig, inputEntities);

      for (auto inputEntity : inputEntities) {
        auto &inputTransform = world.getComponent<ecs::Transform>(inputEntity);

        float dx = attractTransform.x - inputTransform.x;
        float dy = attractTransform.y - inputTransform.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= attraction.radius && distance > 0.0F) {
          // Attract the player towards the attraction entity (boss projectile)
          // Apply directly to position since InputMovementSystem resets velocity
          float attractionX = (dx / distance) * attraction.force * deltaTime;
          float attractionY = (dy / distance) * attraction.force * deltaTime;

          inputTransform.x += attractionX;
          inputTransform.y += attractionY;
        }
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Attraction>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }
};
} // namespace server

#endif // SERVER_ATTRACTIONSYSTEM_HPP_
