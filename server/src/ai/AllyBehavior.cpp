/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyBehavior.cpp - Implementation of ally behavior systems
*/

#include "../../include/ai/AllyBehavior.hpp"
#include "../../include/ai/AllyAIUtility.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include <cstdlib>
#include <cmath>

namespace server::ai::behavior
{

// ============================================================================
// MovementBehavior
// ============================================================================

MovementBehavior::MovementBehavior() : m_horizontalTimer(0.0f), m_currentXDirection(0.0f)
{
}

void MovementBehavior::update(float deltaTime, ecs::Velocity &allyVelocity, const ecs::Transform &allyTransform,
                               const ecs::Transform &targetTransform)
{
  m_horizontalTimer += deltaTime;

  // Update vertical movement
  allyVelocity.dy = calculateVerticalVelocity(allyTransform, targetTransform);

  // Update horizontal movement (changes direction periodically)
  if (m_horizontalTimer >= utility::HORIZONTAL_CHANGE_INTERVAL) {
    updateHorizontalDirection();
    m_horizontalTimer = 0.0f;
  }

  allyVelocity.dx = calculateHorizontalVelocity();
}

void MovementBehavior::reset()
{
  m_horizontalTimer = 0.0f;
  m_currentXDirection = 0.0f;
}

void MovementBehavior::updateHorizontalDirection()
{
  // Pick random direction: -1 (left), 0 (still), or 1 (right)
  int randomChoice = rand() % 3;
  switch (randomChoice) {
    case 0:
      m_currentXDirection = -1.0f; // Move left
      break;
    case 1:
      m_currentXDirection = 1.0f; // Move right
      break;
    default:
      m_currentXDirection = 0.0f; // Stay still
  }
}

float MovementBehavior::calculateVerticalVelocity(const ecs::Transform &allyTransform,
                                                   const ecs::Transform &targetTransform)
{
  float dy = targetTransform.y - allyTransform.y;

  // Only move vertically if significantly out of alignment
  if (std::abs(dy) > utility::VERTICAL_ALIGNMENT_THRESHOLD) {
    return (dy > 0 ? 1.0f : -1.0f) * utility::ALLY_SPEED;
  }

  return 0.0f;
}

float MovementBehavior::calculateHorizontalVelocity()
{
  return m_currentXDirection * (utility::ALLY_SPEED * utility::HORIZONTAL_SPEED_MULTIPLIER);
}

// ============================================================================
// ShootingBehavior
// ============================================================================

ShootingBehavior::ShootingBehavior() : m_shootingTimer(0.0f)
{
}

void ShootingBehavior::update(float deltaTime, ecs::World &world, ecs::Entity allyEntity,
                               const ecs::Transform &allyTransform, const ecs::Transform &targetTransform)
{
  m_shootingTimer += deltaTime;

  if (isAlignedForShooting(allyTransform, targetTransform) && m_shootingTimer >= utility::SHOOTING_INTERVAL) {
    shoot(world, allyEntity);
    m_shootingTimer = 0.0f;
  }
}

void ShootingBehavior::reset()
{
  m_shootingTimer = 0.0f;
}

bool ShootingBehavior::isAlignedForShooting(const ecs::Transform &allyTransform,
                                            const ecs::Transform &targetTransform)
{
  float dy = targetTransform.y - allyTransform.y;
  return std::abs(dy) <= utility::VERTICAL_ALIGNMENT_THRESHOLD;
}

void ShootingBehavior::shoot(ecs::World &world, ecs::Entity allyEntity)
{
  // Shoot in direction (1.0, 0.0) - right, assuming enemies are to the right
  ecs::ShootEvent shootEvent(allyEntity, 1.0f, 0.0f);
  world.emitEvent(shootEvent);
}

// ============================================================================
// AnimationBehavior
// ============================================================================

void AnimationBehavior::update(ecs::World &world, ecs::Entity allyEntity, const ecs::Velocity &velocity)
{
  if (!world.hasComponent<ecs::Sprite>(allyEntity)) {
    return;
  }

  auto &sprite = world.getComponent<ecs::Sprite>(allyEntity);
  sprite.currentFrame = selectAnimationFrame(velocity);
}

int AnimationBehavior::selectAnimationFrame(const ecs::Velocity &velocity)
{
  if (velocity.dy < -utility::ANIMATION_VELOCITY_THRESHOLD) {
    return utility::ANIMATION_FRAME_UP;
  }

  if (velocity.dy > utility::ANIMATION_VELOCITY_THRESHOLD) {
    return utility::ANIMATION_FRAME_DOWN;
  }

  return utility::ANIMATION_FRAME_NEUTRAL;
}

} // namespace server::ai::behavior
