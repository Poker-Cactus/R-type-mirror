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
        continue; // Skip if no Animation component (should be added by NetworkReceiveSystem)
      }

      const auto &velocity = world.getComponent<ecs::Velocity>(entity);
      auto &animation = world.getComponent<ecs::Animation>(entity);

      // Determine target frame based on vertical velocity with hysteresis
      // Only change frame if velocity clearly exceeds thresholds
      constexpr float MAX_TILT_THRESHOLD = 150.0f; // pixels/sec for full tilt
      constexpr float MIN_TILT_THRESHOLD = 80.0f; // pixels/sec for slight tilt

      std::uint32_t newTargetFrame = animation.targetFrame; // Start with current

      // Only change if velocity is clearly in a new range
      if (velocity.dy > MAX_TILT_THRESHOLD) {
        newTargetFrame = 0; // Max down
      } else if (velocity.dy > MIN_TILT_THRESHOLD) {
        newTargetFrame = 1; // Down transition
      } else if (velocity.dy < -MAX_TILT_THRESHOLD) {
        newTargetFrame = 4; // Max up
      } else if (velocity.dy < -MIN_TILT_THRESHOLD) {
        newTargetFrame = 3; // Up transition
      } else if (std::abs(velocity.dy) < 20.0f) {
        // Only snap to neutral if velocity is very close to zero
        newTargetFrame = 2; // Neutral
      }
      // Otherwise keep current frame (hysteresis)

      animation.targetFrame = newTargetFrame;
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Velocity>());
    sig.set(ecs::getComponentId<ecs::PlayerIndex>()); // Only applies to player ships
    return sig;
  }
};

} // namespace client

#endif // CLIENT_ANIMATION_DRIVER_SYSTEM_HPP_
