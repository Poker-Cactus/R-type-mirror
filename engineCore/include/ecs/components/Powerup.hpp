/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Powerup.hpp - Component for collectible power-ups
*/

#pragma once

#include "IComponent.hpp"
#include <cstdint>

namespace ecs
{

/**
 * @brief Types of power-ups available in the game
 */
enum class PowerupType : std::uint8_t {
  DRONE = 0, // Spawns a follower drone that assists the player
  BUBBLE = 1,
  BUBBLE_TRIPLE = 2,
  BUBBLE_RUBAN = 3
};

/**
 * @brief Component that marks an entity as a collectible power-up
 *
 * When a player collides with an entity that has this component,
 * the PowerupSystem applies the power-up effect and destroys the entity.
 */
struct Powerup : public ecs::IComponent {
  PowerupType type = PowerupType::DRONE;

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["type"] = static_cast<std::uint8_t>(type);
    return json;
  }

  static Powerup fromJson(const nlohmann::json &json)
  {
    Powerup powerup;
    powerup.type = static_cast<PowerupType>(json.value("type", static_cast<std::uint8_t>(0)));
    return powerup;
  }
};

} // namespace ecs
