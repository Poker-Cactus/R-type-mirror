/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Transform.hpp
*/

#ifndef ECS_COMPONENTS_TRANSFORM_HPP_
#define ECS_COMPONENTS_TRANSFORM_HPP_

#include "IComponent.hpp"

namespace ecs
{

struct Transform : public IComponent {
  float x = 0.F;
  float y = 0.F;
  float rotation = 0.F;
  float scale = 1.F;

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {{"x", x}, {"y", y}, {"rotation", rotation}, {"scale", scale}};
  }
};

} // namespace ecs

#endif // ECS_COMPONENTS_TRANSFORM_HPP_
