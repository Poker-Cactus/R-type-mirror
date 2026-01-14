/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Animation.hpp - Component for client-side animation state
*/

#ifndef ECS_COMPONENTS_ANIMATION_HPP_
#define ECS_COMPONENTS_ANIMATION_HPP_

#include "IComponent.hpp"
#include <cstdint>

namespace ecs
{

/**
 * @brief Animation component - client-side animation control
 *
 * DESIGN PRINCIPLE:
 * This component is ONLY used on the client for per-entity animation.
 * The server never touches this. AnimationDriverSystem sets targetFrame
 * based on game state (velocity, etc.), and AnimationSystem interpolates
 * currentFrame toward targetFrame.
 */
struct Animation {
  std::uint32_t targetFrame = 2; // Target frame (set by driver system)
  float currentFrame = 2.0f; // Current interpolated frame (used for rendering)
  float interpolationSpeed = 5.0f; // How fast to interpolate (frames per second)
};

} // namespace ecs

#endif // ECS_COMPONENTS_ANIMATION_HPP_
