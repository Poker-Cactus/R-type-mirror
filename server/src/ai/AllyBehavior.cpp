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
                               const ecs::Transform &targetTransform, float speedMultiplier, 
                               float horizontalSpeedMultiplier)
{
  m_horizontalTimer += deltaTime;

  // Update vertical movement
  allyVelocity.dy = calculateVerticalVelocity(allyTransform, targetTransform, speedMultiplier);

  // Update horizontal movement (changes direction periodically)
  if (m_horizontalTimer >= utility::HORIZONTAL_CHANGE_INTERVAL) {
    updateHorizontalDirection();
    m_horizontalTimer = 0.0f;
  }

  allyVelocity.dx = calculateHorizontalVelocity(horizontalSpeedMultiplier);
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
                                                   const ecs::Transform &targetTransform, float speedMultiplier)
{
  float dy = targetTransform.y - allyTransform.y;

  // Only move vertically if significantly out of alignment
  if (std::abs(dy) > utility::VERTICAL_ALIGNMENT_THRESHOLD) {
    return (dy > 0 ? 1.0f : -1.0f) * utility::ALLY_SPEED * speedMultiplier;
  }

  return 0.0f;
}

float MovementBehavior::calculateHorizontalVelocity(float horizontalSpeedMultiplier)
{
  return m_currentXDirection * (utility::ALLY_SPEED * utility::HORIZONTAL_SPEED_MULTIPLIER * horizontalSpeedMultiplier);
}

// ============================================================================
// ShootingBehavior
// ============================================================================

ShootingBehavior::ShootingBehavior() : m_shootingTimer(0.0f)
{
}

void ShootingBehavior::update(float deltaTime, ecs::World &world, ecs::Entity allyEntity,
                               const ecs::Transform &allyTransform, const ecs::Transform &targetTransform,
                               float shootingInterval, float shootingAccuracy)
{
  m_shootingTimer += deltaTime;

  if (isAlignedForShooting(allyTransform, targetTransform, shootingAccuracy) && m_shootingTimer >= shootingInterval) {
    shoot(world, allyEntity);
    m_shootingTimer = 0.0f;
  }
}

void ShootingBehavior::reset()
{
  m_shootingTimer = 0.0f;
}

bool ShootingBehavior::isAlignedForShooting(const ecs::Transform &allyTransform,
                                            const ecs::Transform &targetTransform, float shootingAccuracy)
{
  float dy = targetTransform.y - allyTransform.y;
  float alignmentError = std::abs(dy);
  
  // Perfect accuracy: only shoot when perfectly aligned
  if (shootingAccuracy >= 1.0f) {
    return alignmentError <= utility::VERTICAL_ALIGNMENT_THRESHOLD;
  }
  
  // Lower accuracy: shoot based on probability
  // At accuracy 0.0, shoot randomly regardless of alignment
  // At accuracy 1.0, only shoot when perfectly aligned
  float maxAllowedError = utility::VERTICAL_ALIGNMENT_THRESHOLD / shootingAccuracy;
  if (alignmentError <= maxAllowedError) {
    return true;
  }
  
  // For errors beyond the threshold but within a wider range, shoot with reduced probability
  float widerThreshold = maxAllowedError * 2.0f;
  if (alignmentError <= widerThreshold) {
    float probability = 1.0f - ((alignmentError - maxAllowedError) / maxAllowedError);
    return (rand() / static_cast<float>(RAND_MAX)) < probability;
  }
  
  return false;
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
