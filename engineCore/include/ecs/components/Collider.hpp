/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Collider.hpp - Collision detection component
*/

#ifndef ENGINECORE_ECS_COMPONENTS_COLLIDER_HPP
#define ENGINECORE_ECS_COMPONENTS_COLLIDER_HPP

#include "IComponent.hpp"

namespace ecs
{

/**
 * @brief Collider component for collision detection
 */
struct Collider : public IComponent {
  enum class Shape { BOX, CIRCLE };
  
  Shape shape;
  float width;
  float height;
  float radius;
  bool isTrigger;     // If true, doesn't apply physics, just emits events
  
  Collider(float w = 32.0F, float h = 32.0F)
    : shape(Shape::BOX), width(w), height(h), radius(16.0F), isTrigger(false) {}
  
  [[nodiscard]] nlohmann::json toJson() const override
  {
    return {
      {"shape", shape == Shape::BOX ? "box" : "circle"},
      {"width", width},
      {"height", height},
      {"radius", radius},
      {"isTrigger", isTrigger}
    };
  }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_COLLIDER_HPP
