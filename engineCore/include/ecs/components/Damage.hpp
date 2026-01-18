/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Damage.hpp - Per-entity damage value
*/

#ifndef ECS_COMPONENTS_DAMAGE_HPP_
#define ECS_COMPONENTS_DAMAGE_HPP_

#include "IComponent.hpp"
#include <nlohmann/json.hpp>

namespace ecs
{

/**
 * @brief Damage component - overrides default damage when colliding
 */
struct Damage : public IComponent {
  int amount = 0;

  [[nodiscard]] nlohmann::json toJson() const override { return {{"amount", amount}}; }
};

} // namespace ecs

#endif // ECS_COMPONENTS_DAMAGE_HPP_