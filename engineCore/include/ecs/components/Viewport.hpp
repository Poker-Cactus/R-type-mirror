/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Viewport.hpp - Client viewport dimensions (used for server-side clamping)
*/

#ifndef ECS_COMPONENTS_VIEWPORT_HPP_
#define ECS_COMPONENTS_VIEWPORT_HPP_

#include <cstdint>

namespace ecs
{

struct Viewport {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
};

} // namespace ecs

#endif // ECS_COMPONENTS_VIEWPORT_HPP_
