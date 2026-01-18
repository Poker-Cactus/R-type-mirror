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
  std::uint32_t startFrame = 0; // First frame of animation sequence
  std::uint32_t endFrame = 0; // Last frame of animation sequence
  bool loop = true; // Should the animation loop
  float frameTime = 0.1f; // Time per frame in seconds
  float animationTimer = 0.0f; // Internal timer for animation
  bool reverseAnimation = false; // Play animation in reverse order

  // Spritesheet row and offset support
  std::uint32_t row = 0; // Which row (0-indexed) to use in the spritesheet
  std::uint32_t offsetX = 0; // Horizontal pixel offset (skip columns)
  std::uint32_t offsetY = 0; // Vertical pixel offset (skip rows)

  // Flip support
  bool flipX = false; // Flip horizontally
  bool flipY = false; // Flip vertically

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["spriteId"] = spriteId;
    json["width"] = width;
    json["height"] = height;
    json["animated"] = animated;
    json["frameCount"] = frameCount;
    // currentFrame is included for server-controlled sprites like turrets
    json["currentFrame"] = currentFrame;
    json["startFrame"] = startFrame;
    json["endFrame"] = endFrame;
    json["loop"] = loop;
    json["frameTime"] = frameTime;
    json["reverseAnimation"] = reverseAnimation;
    json["row"] = row;
    json["offsetX"] = offsetX;
    json["offsetY"] = offsetY;
    json["flipX"] = flipX;
    json["flipY"] = flipY;
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
    // currentFrame can be deserialized for server-controlled sprites
    sprite.currentFrame = json.value("currentFrame", json.value("startFrame", 0u));
    sprite.startFrame = json.value("startFrame", 0u);
    sprite.endFrame = json.value("endFrame", 0u);
    sprite.loop = json.value("loop", true);
    sprite.frameTime = json.value("frameTime", 0.1f);
    sprite.reverseAnimation = json.value("reverseAnimation", false);
    sprite.row = json.value("row", 0u);
    sprite.offsetX = json.value("offsetX", 0u);
    sprite.offsetY = json.value("offsetY", 0u);
    sprite.flipX = json.value("flipX", false);
    sprite.flipY = json.value("flipY", false);
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
constexpr std::uint32_t ENEMY_YELLOW = 6;
constexpr std::uint32_t ENEMY_WALKER = 7;
constexpr std::uint32_t WALKER_PROJECTILE = 8;
constexpr std::uint32_t ENEMY_ROBOT = 9;
constexpr std::uint32_t ROBOT_PROJECTILE = 10;
constexpr std::uint32_t BUBBLE_TRIPLE = 11;
constexpr std::uint32_t BUBBLE_RUBAN1 = 12;
constexpr std::uint32_t BUBBLE_RUBAN2 = 13;
constexpr std::uint32_t BUBBLE_RUBAN3 = 14;
constexpr std::uint32_t TRIPLE_PROJECTILE = 15;
constexpr std::uint32_t RUBAN1_PROJECTILE = 16;
constexpr std::uint32_t RUBAN2_PROJECTILE = 17;
constexpr std::uint32_t RUBAN3_PROJECTILE = 18;
constexpr std::uint32_t RUBAN4_PROJECTILE = 19;
constexpr std::uint32_t RUBAN5_PROJECTILE = 20;
constexpr std::uint32_t RUBAN6_PROJECTILE = 21;
constexpr std::uint32_t RUBAN7_PROJECTILE = 22;
constexpr std::uint32_t RUBAN8_PROJECTILE = 23;
constexpr std::uint32_t RUBAN9_PROJECTILE = 24;
constexpr std::uint32_t RUBAN10_PROJECTILE = 25;
constexpr std::uint32_t RUBAN11_PROJECTILE = 26;
constexpr std::uint32_t RUBAN12_PROJECTILE = 27;
constexpr std::uint32_t RUBAN13_PROJECTILE = 28;
constexpr std::uint32_t RUBAN14_PROJECTILE = 29;
constexpr std::uint32_t RUBAN15_PROJECTILE = 30;
constexpr std::uint32_t RUBAN16_PROJECTILE = 31;
constexpr std::uint32_t RUBAN17_PROJECTILE = 32;
constexpr std::uint32_t RUBAN18_PROJECTILE = 33;
constexpr std::uint32_t RUBAN19_PROJECTILE = 34;
constexpr std::uint32_t RUBAN20_PROJECTILE = 35;
constexpr std::uint32_t RUBAN21_PROJECTILE = 36;
constexpr std::uint32_t RUBAN22_PROJECTILE = 37;
constexpr std::uint32_t RUBAN23_PROJECTILE = 38;
constexpr std::uint32_t RUBAN24_PROJECTILE = 39;
constexpr std::uint32_t DRONE = 40;
constexpr std::uint32_t BUBBLE = 41;
constexpr std::uint32_t CHARGED_PROJECTILE = 42;
constexpr std::uint32_t LOADING_SHOT = 43;
constexpr std::uint32_t BOSS_DOBKERATOP = 44;
constexpr std::uint32_t BOSS_DOBKERATOP_SHOOT = 45;
constexpr std::uint32_t BOSS_BROCOLIS = 46;
constexpr std::uint32_t BOSS_BROCOLIS_SHOOT = 47;
constexpr std::uint32_t BOSS_BROCOLIS_ECLOSION = 48;
constexpr std::uint32_t BOSS_GOBLINS = 49;
constexpr std::uint32_t BUBBLE_RUBAN_BACK1 = 50;
constexpr std::uint32_t BUBBLE_RUBAN_BACK2 = 51;
constexpr std::uint32_t BUBBLE_RUBAN_BACK3 = 52;
constexpr std::uint32_t BUBBLE_RUBAN_BACK4 = 53;
constexpr std::uint32_t BUBBLE_RUBAN_MIDDLE1 = 54;
constexpr std::uint32_t BUBBLE_RUBAN_MIDDLE2 = 55;
constexpr std::uint32_t BUBBLE_RUBAN_MIDDLE3 = 56;
constexpr std::uint32_t BUBBLE_RUBAN_MIDDLE4 = 57;
constexpr std::uint32_t BUBBLE_RUBAN_FRONT1 = 58;
constexpr std::uint32_t BUBBLE_RUBAN_FRONT2 = 59;
constexpr std::uint32_t BUBBLE_RUBAN_FRONT3 = 60;
constexpr std::uint32_t BUBBLE_RUBAN_FRONT4 = 61;
constexpr std::uint32_t TRIPLE_PROJECTILE_RIGHT = 62;
constexpr std::uint32_t TRIPLE_PROJECTILE_UP = 63;
constexpr std::uint32_t TRIPLE_PROJECTILE_DOWN = 64;
constexpr std::uint32_t BOSS_GOBLINS_BABY = 65;
constexpr std::uint32_t DEATH_ANIM = 66; // previously collided with BOSS_GOBLINS_BABY, assign unique id
constexpr std::uint32_t BOSS_GREEN_MOTHERSHIP = 67;
constexpr std::uint32_t BOSS_GREEN_MOTHERSHIP_SHOOT = 68;
constexpr std::uint32_t BOSS_GREEN_MOTHERSHIP_ECLOSION = 69;
constexpr std::uint32_t BOSS_GREEN_MOTHERSHIP_TURRET = 70;
constexpr std::uint32_t BOSS_GREEN_MOTHERSHIP_TURRET_SHOT = 71;
constexpr std::uint32_t BOSS_GREEN_MOTHERSHIP_BOOSTER = 72;
} // namespace SpriteId

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_SPRITE_HPP
