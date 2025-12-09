/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Sprite.hpp
*/

#ifndef ENGINECORE_ECS_COMPONENTS_SPRITE_HPP
#define ENGINECORE_ECS_COMPONENTS_SPRITE_HPP

#include "IComponent.hpp"

namespace ecs
{
struct Sprite : public IComponent {
  int textureId;
  int width;
  int height;

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {{"textureId", textureId}, {"width", width}, {"height", height}};
  }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_SPRITE_HPP