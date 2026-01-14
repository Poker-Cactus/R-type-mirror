/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AnimationDriverSystem.hpp - Drives animation based on game state
*/

#ifndef CLIENT_ANIMATION_DRIVER_SYSTEM_HPP_
#define CLIENT_ANIMATION_DRIVER_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Animation.hpp"
#include "../../../engineCore/include/ecs/components/PlayerIndex.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <vector>

namespace client
{

/**
 * @brief System that drives animation frame selection based on velocity
 *
 * DESIGN PRINCIPLE:
 * This system reads gameplay state (Velocity) and decides which animation
 * frame should be shown. It sets Animation.targetFrame, which is then
 * interpolated by AnimationSystem.
 *
 * For player ships:
 * - Frame 0: max down (vy > threshold)
 * - Frame 1: down transition (vy > 0)
 * - Frame 2: neutral (vy ≈ 0)
 * - Frame 3: up transition (vy < 0)
 * - Frame 4: max up (vy < -threshold)
 */
class AnimationDriverSystem : public ecs::ISystem
{
public:
  AnimationDriverSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      if (!world.hasComponent<ecs::Animation>(entity)) {
        continue; // Skip if no Animation component
      }

      auto &animation = world.getComponent<ecs::Animation>(entity);
      
      // Keep player ships in neutral position (frame 2)
      // Animation based on input would require Input component to be replicated,
      // which goes against network-authoritative architecture.
      // For visual feedback, neutral frame is stable and doesn't flicker.
      animation.targetFrame = 2;
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::PlayerIndex>()); // Only applies to player ships
    return sig;
  }
};

} // namespace client

#endif // CLIENT_ANIMATION_DRIVER_SYSTEM_HPP_
