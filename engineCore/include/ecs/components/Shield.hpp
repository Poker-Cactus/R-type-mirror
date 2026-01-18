/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Shield.hpp - Component for shield entities linked to a parent
*/

#ifndef ECS_COMPONENTS_SHIELD_HPP_
#define ECS_COMPONENTS_SHIELD_HPP_

#include "../Entity.hpp"
#include "IComponent.hpp"
#include <nlohmann/json_fwd.hpp>

namespace ecs
{

/**
 * @brief Component that marks an entity as a shield linked to a parent entity
 */
struct Shield : public ecs::IComponent {
  Entity parent = 0;

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["parent"] = parent;
    return json;
  }

  static Shield fromJson(const nlohmann::json &json)
  {
    Shield shield;
    shield.parent = json.value("parent", static_cast<Entity>(0));
    return shield;
  }
};

} // namespace ecs

#endif // ECS_COMPONENTS_SHIELD_HPP_
