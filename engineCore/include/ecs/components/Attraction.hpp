/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Attraction.hpp
*/

#ifndef ENGINECORE_ECS_COMPONENTS_ATTRACTION_HPP
#define ENGINECORE_ECS_COMPONENTS_ATTRACTION_HPP

#include "../Entity.hpp"
#include "IComponent.hpp"
#include <nlohmann/json.hpp>

namespace ecs
{

/**
 * @brief Component that tracks an entity's charging state
 * Used to display loading animation during charged shot charging
 */
struct Attraction : public IComponent {
  float force = 0.0F; // Attraction force magnitude
  float radius = 0.0F; // Effective radius of attraction 

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["force"] = force;
    json["radius"] = radius;
    return json;
  }

  static Attraction fromJson(const nlohmann::json &json)
  {
    Attraction attraction;
    attraction.force = json.value("force", 1000.0F);
    attraction.radius = json.value("radius", 100.0F);
    return attraction;
  }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_ATTRACTION_HPP
