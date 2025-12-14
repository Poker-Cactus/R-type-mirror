/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** GunOffset.hpp - Configuration component for projectile spawn offset
*/

#ifndef ENGINECORE_ECS_COMPONENTS_GUN_OFFSET_HPP
#define ENGINECORE_ECS_COMPONENTS_GUN_OFFSET_HPP

#include "IComponent.hpp"

namespace ecs
{

/**
 * @brief Configuration component for projectile muzzle offset.
 *
 * Entities with this component spawn projectiles offset from their origin.
 * Systems use this to determine projectile spawn position WITHOUT checking entity identity.
 *
 * Example: Player ships might have GunOffset{20.0f}, enemies have none or different values.
 * This is capability-based design: "What can this entity do?" not "What kind is it?"
 */
struct GunOffset : public IComponent {
  float x = 0.0F; ///< Forward offset distance (multiplied by direction)

  GunOffset() = default;
  explicit GunOffset(float offset) : x(offset) {}

  [[nodiscard]] nlohmann::json toJson() const override { return {{"x", x}}; }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_GUN_OFFSET_HPP
