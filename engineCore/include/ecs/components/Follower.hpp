/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Follower.hpp - Component for entities that follow a parent entity
*/

#pragma once

#include "../Entity.hpp"
#include "IComponent.hpp"

namespace ecs
{

/**
 * @brief Component that makes an entity follow another entity (parent)
 *
 * Used for drones, satellites, or any entity that should maintain
 * a position relative to another entity.
 */
struct Follower : public ecs::IComponent {
  Entity parent = 0; // The entity to follow
  float offsetX = 50.0f; // X offset from parent position
  float offsetY = 0.0f; // Y offset from parent position
  float smoothing = 5.0f; // Smoothing factor for movement (higher = faster catch up)

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["parent"] = parent;
    json["offsetX"] = offsetX;
    json["offsetY"] = offsetY;
    json["smoothing"] = smoothing;
    return json;
  }

  static Follower fromJson(const nlohmann::json &json)
  {
    Follower follower;
    follower.parent = json.value("parent", static_cast<Entity>(0));
    follower.offsetX = json.value("offsetX", 50.0f);
    follower.offsetY = json.value("offsetY", 0.0f);
    follower.smoothing = json.value("smoothing", 5.0f);
    return follower;
  }
};

} // namespace ecs
