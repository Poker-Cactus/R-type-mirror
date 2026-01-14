/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ShipStats.hpp - Component for ship gameplay characteristics
*/

#ifndef ECS_COMPONENTS_SHIPSTATS_HPP_
#define ECS_COMPONENTS_SHIPSTATS_HPP_

#include "IComponent.hpp"
#include <nlohmann/json.hpp>

namespace ecs
{

/**
 * @brief Ship statistics component - defines gameplay characteristics
 *
 * DESIGN PRINCIPLE:
 * Ship identity is expressed through different stat values, not through
 * entity types or markers. Different ships = different ShipStats values.
 *
 * Ship types (conceptual):
 * - Default: balanced stats
 * - Fast: high moveSpeed, low maxHP
 * - Tank: high maxHP, low moveSpeed
 * - Sniper: high damage, low fireRate
 */
struct ShipStats : public IComponent {
  int maxHP = 100; // Maximum hit points
  float moveSpeed = 200.0f; // Movement speed in pixels/second
  float fireRate = 0.25f; // Time between shots in seconds
  int damage = 10; // Damage dealt by projectiles

  ShipStats() = default;
  ShipStats(int hp, float speed, float rate, int dmg) : maxHP(hp), moveSpeed(speed), fireRate(rate), damage(dmg) {}

  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {{"maxHP", maxHP}, {"moveSpeed", moveSpeed}, {"fireRate", fireRate}, {"damage", damage}};
  }

  static ShipStats fromJson(const nlohmann::json &json)
  {
    ShipStats stats;
    stats.maxHP = json.value("maxHP", 100);
    stats.moveSpeed = json.value("moveSpeed", 200.0f);
    stats.fireRate = json.value("fireRate", 0.25f);
    stats.damage = json.value("damage", 10);
    return stats;
  }
};

} // namespace ecs

#endif // ECS_COMPONENTS_SHIPSTATS_HPP_
