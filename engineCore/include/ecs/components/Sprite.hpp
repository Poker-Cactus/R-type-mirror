/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Sprite.hpp
*/

#ifndef ENGINECORE_ECS_COMPONENTS_SPRITE_HPP
#define ENGINECORE_ECS_COMPONENTS_SPRITE_HPP

#include "IComponent.hpp"
#include <cstdint>
#include <nlohmann/json_fwd.hpp>

namespace ecs
{

/**
 * @brief Sprite component - defines visual representation
 *
 * DESIGN PRINCIPLE:
 * Visual identity is replicated data decided by the server.
 * The client is a pure renderer and must never infer game roles.
 *
 * spriteId is an abstract identifier used by the client to map to textures.
 * The server assigns this at entity creation time - it is DATA, not LOGIC.
 */
struct Sprite : public IComponent {
  static constexpr std::uint32_t DEFAULT_SPRITE_SIZE = 32U;
  
  std::uint32_t spriteId = 0U;
  std::uint32_t width = DEFAULT_SPRITE_SIZE;
  std::uint32_t height = DEFAULT_SPRITE_SIZE;

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["spriteId"] = spriteId;
    json["width"] = width;
    json["height"] = height;
    return json;
  }

  static Sprite fromJson(const nlohmann::json &json)
  {
    Sprite sprite;
    sprite.spriteId = json.value("spriteId", 0U);
    sprite.width = json.value("width", DEFAULT_SPRITE_SIZE);
    sprite.height = json.value("height", DEFAULT_SPRITE_SIZE);
    return sprite;
  }
};

// Abstract sprite identifiers
// These are used by the server to assign visual identity
// The client maps these to actual texture files
namespace SpriteId
{
constexpr std::uint32_t PLAYER_SHIP = 1;
constexpr std::uint32_t ENEMY_SHIP = 2;
constexpr std::uint32_t PROJECTILE = 3;
constexpr std::uint32_t EXPLOSION = 4;
constexpr std::uint32_t POWERUP = 5;
} // namespace SpriteId

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_SPRITE_HPP