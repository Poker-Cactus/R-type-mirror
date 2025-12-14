/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EnemyAI.hpp - Marker component for AI-driven enemies
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ROLES_ENEMY_AI_HPP
#define ENGINECORE_ECS_COMPONENTS_ROLES_ENEMY_AI_HPP

#include "../IComponent.hpp"

namespace ecs
{

struct EnemyAI : public IComponent {
  [[nodiscard]] nlohmann::json toJson() const override { return nlohmann::json::object(); }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ROLES_ENEMY_AI_HPP
