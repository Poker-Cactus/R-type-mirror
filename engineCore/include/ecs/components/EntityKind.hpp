/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EntityKind.hpp - Simple kind tag for rendering/logic
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ENTITY_KIND_HPP
#define ENGINECORE_ECS_COMPONENTS_ENTITY_KIND_HPP

#include "IComponent.hpp"

namespace ecs
{

struct EntityKind : public IComponent {
  enum class Kind : unsigned char { UNKNOWN = 0, PLAYER, ENEMY, PROJECTILE };

  Kind kind = Kind::UNKNOWN;

  EntityKind() = default;
  explicit EntityKind(Kind k) : kind(k) {}

  [[nodiscard]] nlohmann::json toJson() const override
  {
    const char *s = "unknown";
    switch (kind) {
    case Kind::PLAYER:
      s = "player";
      break;
    case Kind::ENEMY:
      s = "enemy";
      break;
    case Kind::PROJECTILE:
      s = "projectile";
      break;
    default:
      s = "unknown";
      break;
    }
    return {{"kind", s}};
  }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ENTITY_KIND_HPP
