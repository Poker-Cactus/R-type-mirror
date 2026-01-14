/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AnimationSystem.hpp - Interpolates animation frames
*/

#ifndef CLIENT_ANIMATION_SYSTEM_HPP_
#define CLIENT_ANIMATION_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Animation.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <vector>

namespace client
{

/**
 * @brief System that interpolates animation frames smoothly
 *
 * DESIGN PRINCIPLE:
 * This system reads Animation.targetFrame (set by AnimationDriverSystem)
 * and interpolates Animation.currentFrame toward it. The rendering system
 * uses floor(currentFrame) to determine which sprite frame to draw.
 *
 * This creates smooth, per-entity animation without global state.
 */
class AnimationSystem : public ecs::ISystem
{
public:
  AnimationSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      auto &animation = world.getComponent<ecs::Animation>(entity);
      auto &sprite = world.getComponent<ecs::Sprite>(entity);

      // Interpolate currentFrame toward targetFrame
      float diff = static_cast<float>(animation.targetFrame) - animation.currentFrame;
      float step = animation.interpolationSpeed * deltaTime;

      if (std::abs(diff) < step) {
        // Snap to target if close enough
        animation.currentFrame = static_cast<float>(animation.targetFrame);
      } else {
        // Move toward target
        animation.currentFrame += (diff > 0.0f) ? step : -step;
      }

      // Update sprite frame for rendering
      sprite.currentFrame = static_cast<std::uint32_t>(std::floor(animation.currentFrame));
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Animation>());
    sig.set(ecs::getComponentId<ecs::Sprite>());
    return sig;
  }
};

} // namespace client

#endif // CLIENT_ANIMATION_SYSTEM_HPP_
