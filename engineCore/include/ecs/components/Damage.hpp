/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Damage.hpp - Component for entities that can deal damage
*/

#ifndef ECS_COMPONENTS_DAMAGE_HPP_
#define ECS_COMPONENTS_DAMAGE_HPP_

#include "IComponent.hpp"
#include <nlohmann/json.hpp>

namespace ecs
{

/**
 * @brief Damage component - defines how much damage an entity deals on collision
 *
 * DESIGN PRINCIPLE:
 * Entities that can damage others (projectiles, enemies on contact) have this component.
 * No need to check "what type" of entity - just check if it has Damage component.
 */
struct Damage : public IComponent {
  int amount = 10; // Damage amount dealt on collision

  [[nodiscard]] nlohmann::json toJson() const override { return {{"amount", amount}}; }

  static Damage fromJson(const nlohmann::json &json)
  {
    Damage dmg;
    dmg.amount = json.value("amount", 10);
    return dmg;
  }
};

} // namespace ecs

#endif // ECS_COMPONENTS_DAMAGE_HPP_
