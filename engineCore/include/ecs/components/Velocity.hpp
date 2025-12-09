/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Velocity.hpp
*/

#ifndef ENGINECORE_ECS_COMPONENTS_VELOCITY_HPP
#define ENGINECORE_ECS_COMPONENTS_VELOCITY_HPP

#include "IComponent.hpp"

namespace ecs
{
struct Velocity : public IComponent {
  float dx;
  float dy;

  [[nodiscard]] nlohmann::json toJson() const override { return {{"dx", dx}, {"dy", dy}}; }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_VELOCITY_HPP