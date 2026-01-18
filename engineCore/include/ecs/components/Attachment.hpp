/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Attachment component - tracks parent entity and relative position
*/

#ifndef ECS_ATTACHMENT_HPP_
#define ECS_ATTACHMENT_HPP_

#include "IComponent.hpp"
#include "../Entity.hpp"
#include <nlohmann/json_fwd.hpp>

namespace ecs
{

/**
 * @brief Attachment component - tracks parent entity and relative position
 * Used for turrets and other attached entities that move with their parent
 */
struct Attachment : public IComponent {
  Entity parentId = 0;
  float offsetX = 0.0f;  // X offset from parent position
  float offsetY = 0.0f;  // Y offset from parent position

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["parentId"] = parentId;
    json["offsetX"] = offsetX;
    json["offsetY"] = offsetY;
    return json;
  }

  static Attachment fromJson(const nlohmann::json &json)
  {
    Attachment attachment;
    attachment.parentId = json.value("parentId", 0u);
    attachment.offsetX = json.value("offsetX", 0.0f);
    attachment.offsetY = json.value("offsetY", 0.0f);
    return attachment;
  }
};

} // namespace ecs

#endif // ECS_ATTACHMENT_HPP_
