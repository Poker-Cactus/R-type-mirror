/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** PowerUp.hpp - Marker component for power-up entities
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ROLES_POWER_UP_HPP
#define ENGINECORE_ECS_COMPONENTS_ROLES_POWER_UP_HPP

#include "../IComponent.hpp"

namespace ecs
{

struct PowerUp : public IComponent {
  [[nodiscard]] nlohmann::json toJson() const override { return nlohmann::json::object(); }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ROLES_POWER_UP_HPP
