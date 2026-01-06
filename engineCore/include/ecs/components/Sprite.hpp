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
  std::uint32_t spriteId = 0;
  std::uint32_t width = 32;
  std::uint32_t height = 32;
  
  // Animation fields
  bool animated = false;
  std::uint32_t frameCount = 1;
  std::uint32_t currentFrame = 0;
  std::uint32_t startFrame = 0;    // First frame of animation sequence
  std::uint32_t endFrame = 0;      // Last frame of animation sequence
  float frameTime = 0.1f;          // Time per frame in seconds
  float animationTimer = 0.0f;     // Internal timer for animation
  bool reverseAnimation = false;   // Play animation in reverse order

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["spriteId"] = spriteId;
    json["width"] = width;
    json["height"] = height;
    json["animated"] = animated;
    json["frameCount"] = frameCount;
    // currentFrame is NOT serialized - it's client-side animation state
    json["startFrame"] = startFrame;
    json["endFrame"] = endFrame;
    json["frameTime"] = frameTime;
    json["reverseAnimation"] = reverseAnimation;
    return json;
  }

  static Sprite fromJson(const nlohmann::json &json)
  {
    Sprite sprite;
    sprite.spriteId = json.value("spriteId", 0u);
    sprite.width = json.value("width", 32u);
    sprite.height = json.value("height", 32u);
    sprite.animated = json.value("animated", false);
    sprite.frameCount = json.value("frameCount", 1u);
    // currentFrame is NOT deserialized - client manages it locally
    // If it's a new sprite, initialize currentFrame to startFrame
    sprite.startFrame = json.value("startFrame", 0u);
    sprite.endFrame = json.value("endFrame", 0u);
    sprite.currentFrame = json.value("startFrame", 0u); // Initialize to startFrame
    sprite.frameTime = json.value("frameTime", 0.1f);
    sprite.reverseAnimation = json.value("reverseAnimation", false);
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