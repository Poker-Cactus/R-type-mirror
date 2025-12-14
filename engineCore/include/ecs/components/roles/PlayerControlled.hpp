/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** PlayerControlled.hpp - Marker component for player-controlled entities
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ROLES_PLAYER_CONTROLLED_HPP
#define ENGINECORE_ECS_COMPONENTS_ROLES_PLAYER_CONTROLLED_HPP

#include "../IComponent.hpp"

namespace ecs
{

struct PlayerControlled : public IComponent {
  [[nodiscard]] nlohmann::json toJson() const override { return nlohmann::json::object(); }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ROLES_PLAYER_CONTROLLED_HPP
