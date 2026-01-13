/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Charging.hpp - Component to track charging state
*/

#ifndef ENGINECORE_ECS_COMPONENTS_CHARGING_HPP
#define ENGINECORE_ECS_COMPONENTS_CHARGING_HPP

#include "../Entity.hpp"
#include "IComponent.hpp"
#include <nlohmann/json.hpp>

namespace ecs
{

/**
 * @brief Component that tracks an entity's charging state
 * Used to display loading animation during charged shot charging
 */
struct Charging : public IComponent {
  Entity loadingShotEntity = 0; // Entity ID of the loading shot animation
  float chargeTime = 0.0F; // Time spent charging
  float maxChargeTime = 2.0F; // Maximum charge time
  bool isCharging = false; // Is currently charging

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["loadingShotEntity"] = loadingShotEntity;
    json["chargeTime"] = chargeTime;
    json["maxChargeTime"] = maxChargeTime;
    json["isCharging"] = isCharging;
    return json;
  }

  static Charging fromJson(const nlohmann::json &json)
  {
    Charging charging;
    charging.loadingShotEntity = json.value("loadingShotEntity", 0u);
    charging.chargeTime = json.value("chargeTime", 0.0F);
    charging.maxChargeTime = json.value("maxChargeTime", 2.0F);
    charging.isCharging = json.value("isCharging", false);
    return charging;
  }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_CHARGING_HPP
