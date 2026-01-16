/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyBehavior.cpp - Implementation of ally behavior systems
*/

#include "../../include/ai/AllyBehavior.hpp"
#include "../../include/ai/AllyAIUtility.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/Charging.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include <cstdlib>
#include <cmath>

namespace server::ai::behavior
{

// ============================================================================
// MovementBehavior
// ============================================================================

MovementBehavior::MovementBehavior() : m_horizontalTimer(0.0f), m_currentXDirection(0.0f), m_idleTimer(0.0f), m_idleDuration(0.0f), m_isIdling(false)
{
}

void MovementBehavior::update(float deltaTime, ecs::Velocity &allyVelocity, const ecs::Transform &allyTransform,
                               const ecs::Transform &targetTransform, AIStrength strength)
{
  // Update idle state (only for weak AI)
  updateIdleState(deltaTime, strength);

  // If idling, set velocities to zero but still allow obstacle avoidance to override
  if (m_isIdling) {
    allyVelocity.dx = 0.0f;
    allyVelocity.dy = 0.0f;
    return;
  }

  m_horizontalTimer += deltaTime;

  // Update vertical movement
  allyVelocity.dy = calculateVerticalVelocity(allyTransform, targetTransform, strength);

  // Update horizontal movement (changes direction periodically)
  if (m_horizontalTimer >= utility::HORIZONTAL_CHANGE_INTERVAL) {
    updateHorizontalDirection();
    m_horizontalTimer = 0.0f;
  }

  allyVelocity.dx = calculateHorizontalVelocity(strength);
}

void MovementBehavior::reset()
{
  m_horizontalTimer = 0.0f;
  m_currentXDirection = 0.0f;
  m_idleTimer = 0.0f;
  m_idleDuration = 0.0f;
  m_idleCheckTimer = 0.0f;
  m_isIdling = false;
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
                                                   const ecs::Transform &targetTransform, AIStrength strength)
{
  float baseSpeed = utility::ALLY_SPEED;

  // Apply strength multiplier
  switch (strength) {
    case AIStrength::WEAK:
      baseSpeed *= utility::ALLY_SPEED_WEAK_MULTIPLIER;
      break;
    case AIStrength::STRONG:
      baseSpeed *= utility::ALLY_SPEED_STRONG_MULTIPLIER;
      break;
    case AIStrength::MEDIUM:
    default:
      // No change for medium
      break;
  }

  float dy = targetTransform.y - allyTransform.y;

  // Only move vertically if significantly out of alignment
  if (std::abs(dy) > utility::VERTICAL_ALIGNMENT_THRESHOLD) {
    return (dy > 0 ? 1.0f : -1.0f) * baseSpeed;
  }

  return 0.0f;
}

float MovementBehavior::calculateHorizontalVelocity(AIStrength strength)
{
  float baseSpeed = utility::ALLY_SPEED * utility::HORIZONTAL_SPEED_MULTIPLIER;

  // Apply strength multiplier
  switch (strength) {
    case AIStrength::WEAK:
      baseSpeed *= utility::ALLY_SPEED_WEAK_MULTIPLIER;
      break;
    case AIStrength::STRONG:
      baseSpeed *= utility::ALLY_SPEED_STRONG_MULTIPLIER;
      break;
    case AIStrength::MEDIUM:
    default:
      // No change for medium
      break;
  }

  return m_currentXDirection * baseSpeed;
}

void MovementBehavior::updateIdleState(float deltaTime, AIStrength strength)
{
  // Only weak AI has idle behavior
  if (strength != AIStrength::WEAK) {
    return;
  }

  if (m_isIdling) {
    m_idleTimer += deltaTime;
    if (m_idleTimer >= m_idleDuration) {
      // Exit idle state
      m_isIdling = false;
      m_idleTimer = 0.0f;
      m_idleDuration = 0.0f;
    }
  } else {
    // Increment timer for checking idle every 3 seconds
    m_idleCheckTimer += deltaTime;
    if (m_idleCheckTimer >= 3.0f) {
      m_idleCheckTimer = 0.0f; // Reset timer
      // 30% chance to enter idle state
      float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      if (randomValue < 0.3f) {
        m_isIdling = true;
        m_idleDuration = generateIdleDuration();
        m_idleTimer = 0.0f;
      }
    }
  }
}

float MovementBehavior::generateIdleDuration() const
{
  float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  return utility::IDLE_DURATION_MIN + randomValue * (utility::IDLE_DURATION_MAX - utility::IDLE_DURATION_MIN);
}

// ============================================================================
// ShootingBehavior
// ============================================================================

ShootingBehavior::ShootingBehavior() : m_shootingTimer(0.0f)
{
}

void ShootingBehavior::update(float deltaTime, ecs::World &world, ecs::Entity allyEntity,
                               const ecs::Transform &allyTransform, const ecs::Transform &targetTransform, AIStrength strength)
{
  m_shootingTimer += deltaTime;

  if (isAlignedForShooting(allyTransform, targetTransform, strength) && shouldShoot(strength) && m_shootingTimer >= getShootingInterval(strength)) {
    // Check if strong AI should use charge shot
    if (shouldUseChargeShot(world, allyTransform, strength)) {
      chargeShoot(world, allyEntity);
    } else {
      shoot(world, allyEntity);
    }
    m_shootingTimer = 0.0f;
  }
}

void ShootingBehavior::reset()
{
  m_shootingTimer = 0.0f;
}

bool ShootingBehavior::isAlignedForShooting(const ecs::Transform &allyTransform,
                                            const ecs::Transform &targetTransform, AIStrength strength)
{
  float dy = targetTransform.y - allyTransform.y;
  float threshold;
  switch (strength) {
    case AIStrength::WEAK:
      threshold = utility::VERTICAL_ALIGNMENT_THRESHOLD * 0.6f; // Smaller range for weak AI
      break;
    case AIStrength::MEDIUM:
      threshold = utility::VERTICAL_ALIGNMENT_THRESHOLD; // Normal range
      break;
    case AIStrength::STRONG:
      threshold = utility::VERTICAL_ALIGNMENT_THRESHOLD * 1.4f; // Larger range for strong AI
      break;
    default:
      threshold = utility::VERTICAL_ALIGNMENT_THRESHOLD;
      break;
  }
  return std::abs(dy) <= threshold;
}

void ShootingBehavior::shoot(ecs::World &world, ecs::Entity allyEntity)
{
  // Shoot in direction (1.0, 0.0) - right, assuming enemies are to the right
  ecs::ShootEvent shootEvent(allyEntity, 1.0f, 0.0f);
  world.emitEvent(shootEvent);
}

void ShootingBehavior::chargeShoot(ecs::World &world, ecs::Entity allyEntity)
{
  // Add Charging component if entity doesn't have one
  if (!world.hasComponent<ecs::Charging>(allyEntity)) {
    world.addComponent(allyEntity, ecs::Charging{});
  }
  auto &charging = world.getComponent<ecs::Charging>(allyEntity);

  // Only start charging if not already charging
  if (!charging.isCharging) {
    constexpr float LOADING_OFFSET_X = 130.0F;
    constexpr float LOADING_OFFSET_Y = 0.0F;
    auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
    float transformX = allyTransform.x + LOADING_OFFSET_X;
    float transformY = allyTransform.y + LOADING_OFFSET_Y;

    // Spawn loading shot animation
    ecs::SpawnEntityEvent loadingEvent(ecs::SpawnEntityEvent::EntityType::LOADING_SHOT, transformX, transformY,
                                       allyEntity);
    world.emitEvent(loadingEvent);

    // Find the loading shot entity
    ecs::Entity loadingShotEntity = 0;
    ecs::ComponentSignature ownerSig;
    ownerSig.set(ecs::getComponentId<ecs::Owner>());
    ownerSig.set(ecs::getComponentId<ecs::Sprite>());
    std::vector<ecs::Entity> ownedEntities;
    world.getEntitiesWithSignature(ownerSig, ownedEntities);

    for (auto owned : ownedEntities) {
      const auto &owner = world.getComponent<ecs::Owner>(owned);
      const auto &sprite = world.getComponent<ecs::Sprite>(owned);
      if (owner.ownerId == allyEntity && sprite.spriteId == ecs::SpriteId::LOADING_SHOT) {
        loadingShotEntity = owned;
        break;
      }
    }

    // Add Follower component to make LOADING_SHOT follow the ally
    if (loadingShotEntity != 0 && world.isAlive(loadingShotEntity)) {
      ecs::Follower follower;
      follower.parent = allyEntity;
      follower.offsetX = LOADING_OFFSET_X;
      follower.offsetY = LOADING_OFFSET_Y;
      follower.smoothing = 100.0F; // High smoothing for instant positioning
      world.addComponent(loadingShotEntity, follower);
    }

    // Store charging state in component
    charging.isCharging = true;
    charging.chargeTime = 0.0F;
    charging.maxChargeTime = 0.6F;
    charging.loadingShotEntity = loadingShotEntity;
  }
}

bool ShootingBehavior::shouldUseChargeShot(ecs::World &world, const ecs::Transform &allyTransform, AIStrength strength)
{
  // Only strong AI uses charge shots
  if (strength != AIStrength::STRONG) {
    return false;
  }

  // Count enemies near the ally's Y level
  int enemyCount = countEnemiesAtYLevel(world, allyTransform.y);

  return enemyCount >= utility::CHARGE_SHOT_MIN_ENEMIES;
}

int ShootingBehavior::countEnemiesAtYLevel(ecs::World &world, float targetY) const
{
  int count = 0;

  // Get all entities with Pattern component (enemies)
  std::vector<ecs::Entity> enemies;
  ecs::ComponentSignature enemySig;
  enemySig.set(ecs::getComponentId<ecs::Pattern>());
  enemySig.set(ecs::getComponentId<ecs::Transform>());
  world.getEntitiesWithSignature(enemySig, enemies);

  for (ecs::Entity enemy : enemies) {
    if (!world.isAlive(enemy)) {
      continue;
    }

    auto &enemyTransform = world.getComponent<ecs::Transform>(enemy);
    float dy = std::abs(enemyTransform.y - targetY);

    if (dy <= utility::CHARGE_SHOT_ENEMY_Y_THRESHOLD) {
      count++;
    }
  }

  return count;
}

float ShootingBehavior::getShootingInterval(AIStrength strength) const
{
  float baseInterval = utility::SHOOTING_INTERVAL;

  switch (strength) {
    case AIStrength::WEAK:
      return baseInterval * 2.0f; // Slower shooting
    case AIStrength::STRONG:
      return baseInterval * utility::SHOOTING_INTERVAL_STRONG_MULTIPLIER; // Faster shooting
    case AIStrength::MEDIUM:
    default:
      return baseInterval; // Normal
  }
}

bool ShootingBehavior::shouldShoot(AIStrength strength) const
{
  if (strength != AIStrength::WEAK) {
    return true; // Medium and strong always shoot when aligned
  }

  // For weak AI, sometimes idle instead of shooting
  int randomChoice = rand() % 10;
  return randomChoice < 7; // 70% chance to shoot, 30% to idle
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
