/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** FollowerSystem.hpp - Updates position of entities that follow a parent
*/

#ifndef SERVER_FOLLOWER_SYSTEM_HPP_
#define SERVER_FOLLOWER_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"
#include <array>
#include <unordered_map>
#include <vector>

namespace server
{

/**
 * @brief Animation state for ruban bubble sprites
 * Manages the 4-frame animation sequence based on movement direction
 */
struct RubanBubbleAnimState {
  // Direction indices
  static constexpr int DIR_BACK = 0;
  static constexpr int DIR_MIDDLE = 1;
  static constexpr int DIR_FRONT = 2;

  // 4 frames per direction
  static constexpr std::array<std::uint32_t, 4> BACK_FRAMES = {
    ecs::SpriteId::BUBBLE_RUBAN_BACK1, ecs::SpriteId::BUBBLE_RUBAN_BACK2, ecs::SpriteId::BUBBLE_RUBAN_BACK3,
    ecs::SpriteId::BUBBLE_RUBAN_BACK4};
  static constexpr std::array<std::uint32_t, 4> MIDDLE_FRAMES = {
    ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1, ecs::SpriteId::BUBBLE_RUBAN_MIDDLE2, ecs::SpriteId::BUBBLE_RUBAN_MIDDLE3,
    ecs::SpriteId::BUBBLE_RUBAN_MIDDLE4};
  static constexpr std::array<std::uint32_t, 4> FRONT_FRAMES = {
    ecs::SpriteId::BUBBLE_RUBAN_FRONT1, ecs::SpriteId::BUBBLE_RUBAN_FRONT2, ecs::SpriteId::BUBBLE_RUBAN_FRONT3,
    ecs::SpriteId::BUBBLE_RUBAN_FRONT4};

  int currentDirection = DIR_MIDDLE; // Current direction (back/middle/front)
  int targetDirection = DIR_MIDDLE; // Target direction based on movement
  int frameIndex = 0; // Current frame within direction (0-3)
  float directionTimer = 0.0f; // Timer for direction transitions
  float frameTimer = 0.0f; // Timer for frame animation
  static constexpr float DIRECTION_SPEED = 0.04f; // Time between direction changes
  static constexpr float FRAME_SPEED = 0.08f; // Time between frame changes within direction
};

/**
 * @brief System that updates the position of follower entities
 *
 * Entities with a Follower component will smoothly move towards
 * their target position (parent position + offset).
 */
class FollowerSystem : public ecs::ISystem
{
public:
  FollowerSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (const auto &entity : entities) {
      if (!world.isAlive(entity)) {
        // Clean up animation state for dead entities
        m_rubanAnimStates.erase(entity);
        continue;
      }

      auto &follower = world.getComponent<ecs::Follower>(entity);
      auto &transform = world.getComponent<ecs::Transform>(entity);

      // Check if parent is still alive
      if (!world.isAlive(follower.parent)) {
        // Parent is dead, destroy the follower
        m_rubanAnimStates.erase(entity);
        world.destroyEntity(entity);
        continue;
      }

      // Get parent's transform
      if (!world.hasComponent<ecs::Transform>(follower.parent)) {
        continue;
      }

      const auto &parentTransform = world.getComponent<ecs::Transform>(follower.parent);

      // Calculate target position (parent position + offset)
      float targetX = parentTransform.x + follower.offsetX;
      float targetY = parentTransform.y + follower.offsetY;

      // Check if this is a bubble (instant positioning instead of smooth)
      bool isBubble = false;
      bool isRubanBubble = false;
      if (world.hasComponent<ecs::Sprite>(entity)) {
        const auto &sprite = world.getComponent<ecs::Sprite>(entity);
        // Check for all ruban bubble frame sprites
        bool isRubanSpriteId =
          (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN1 && sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN3) ||
          (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 &&
           sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4);
        isBubble = (sprite.spriteId == ecs::SpriteId::BUBBLE || sprite.spriteId == ecs::SpriteId::BUBBLE_TRIPLE ||
                    isRubanSpriteId);
        isRubanBubble = isRubanSpriteId;
      }

      // Update ruban bubble animation based on parent movement direction
      if (isRubanBubble && world.hasComponent<ecs::Velocity>(follower.parent)) {
        updateRubanBubbleAnimation(world, entity, follower.parent, deltaTime);
      }

      if (isBubble) {
        // Instant positioning for bubbles (no smooth movement)
        transform.x = targetX;
        transform.y = targetY;
      } else {
        // Smoothly interpolate towards target position for other followers (drones)
        float lerpFactor = 1.0f - std::exp(-follower.smoothing * deltaTime);
        transform.x += (targetX - transform.x) * lerpFactor;
        transform.y += (targetY - transform.y) * lerpFactor;
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Follower>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }

private:
  // Animation states for each ruban bubble entity
  std::unordered_map<ecs::Entity, RubanBubbleAnimState> m_rubanAnimStates;

  /**
   * @brief Update ruban bubble animation based on parent movement
   * Animates through the sprite queue (RUBAN1 ↔ RUBAN2 ↔ RUBAN3)
   */
  void updateRubanBubbleAnimation(ecs::World &world, ecs::Entity bubble, ecs::Entity parent, float deltaTime)
  {
    const auto &parentVelocity = world.getComponent<ecs::Velocity>(parent);
    auto &sprite = world.getComponent<ecs::Sprite>(bubble);

    // Initialize animation state if not exists
    if (m_rubanAnimStates.find(bubble) == m_rubanAnimStates.end()) {
      m_rubanAnimStates[bubble] = RubanBubbleAnimState{};
    }

    auto &animState = m_rubanAnimStates[bubble];

    // Determine target direction based on horizontal velocity
    // DIR_BACK = compressed (backward), DIR_MIDDLE = neutral, DIR_FRONT = stretched (forward)
    if (parentVelocity.dx > 10.0f) {
      // Moving forward → FRONT
      animState.targetDirection = RubanBubbleAnimState::DIR_FRONT;
    } else if (parentVelocity.dx < -10.0f) {
      // Moving backward → BACK
      animState.targetDirection = RubanBubbleAnimState::DIR_BACK;
    } else {
      // Idle → MIDDLE (neutral)
      animState.targetDirection = RubanBubbleAnimState::DIR_MIDDLE;
    }

    // Update direction timer - transition between back/middle/front
    animState.directionTimer += deltaTime;
    if (animState.directionTimer >= RubanBubbleAnimState::DIRECTION_SPEED) {
      animState.directionTimer = 0.0f;

      if (animState.currentDirection < animState.targetDirection) {
        animState.currentDirection++;
      } else if (animState.currentDirection > animState.targetDirection) {
        animState.currentDirection--;
      }
      animState.currentDirection = std::max(0, std::min(2, animState.currentDirection));
    }

    // Update frame timer - cycle through 4 frames within current direction
    animState.frameTimer += deltaTime;
    if (animState.frameTimer >= RubanBubbleAnimState::FRAME_SPEED) {
      animState.frameTimer = 0.0f;
      animState.frameIndex = (animState.frameIndex + 1) % 4;
    }

    // Get the appropriate sprite ID based on current direction and frame
    std::uint32_t newSpriteId = 0;
    switch (animState.currentDirection) {
    case RubanBubbleAnimState::DIR_BACK:
      newSpriteId = RubanBubbleAnimState::BACK_FRAMES[animState.frameIndex];
      break;
    case RubanBubbleAnimState::DIR_FRONT:
      newSpriteId = RubanBubbleAnimState::FRONT_FRAMES[animState.frameIndex];
      break;
    case RubanBubbleAnimState::DIR_MIDDLE:
    default:
      newSpriteId = RubanBubbleAnimState::MIDDLE_FRAMES[animState.frameIndex];
      break;
    }

    // Update sprite ID if changed
    if (sprite.spriteId != newSpriteId) {
      sprite.spriteId = newSpriteId;
    }
  }
};

} // namespace server

#endif // SERVER_FOLLOWER_SYSTEM_HPP_
