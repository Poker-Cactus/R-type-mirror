/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Health.hpp
*/

#ifndef ENGINECORE_ECS_COMPONENTS_HEALTH_HPP
#define ENGINECORE_ECS_COMPONENTS_HEALTH_HPP

#include "IComponent.hpp"
namespace ecs
{
struct Health : IComponent {
  int hp;
  int maxHp;

  [[nodiscard]] nlohmann::json toJson() const override { return {{"hp", hp}, {"maxHp", maxHp}}; }
};
} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_HEALTH_HPP