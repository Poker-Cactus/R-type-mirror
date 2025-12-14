/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Projectile.hpp - Marker component for projectile entities
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ROLES_PROJECTILE_HPP
#define ENGINECORE_ECS_COMPONENTS_ROLES_PROJECTILE_HPP

#include "../IComponent.hpp"

namespace ecs
{

struct Projectile : public IComponent {
  [[nodiscard]] nlohmann::json toJson() const override { return nlohmann::json::object(); }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ROLES_PROJECTILE_HPP
