/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyBehavior.hpp - Behavior components for ally AI
*/

#ifndef SERVER_ALLY_BEHAVIOR_HPP_
#define SERVER_ALLY_BEHAVIOR_HPP_

#include "AllyAIUtility.hpp"
#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"

namespace server::ai::behavior
{

/**
 * @brief Handles ally movement decisions
 *
 * Manages:
 * - Vertical alignment with target
 * - Horizontal movement (random but natural)
 * - State tracking for direction changes
 * - Idle behavior for weak AI
 */
class MovementBehavior
{
public:
  MovementBehavior();

  /**
   * @brief Update movement velocities based on target position
   */
  void update(float deltaTime, ecs::Velocity &allyVelocity, const ecs::Transform &allyTransform,
              const ecs::Transform &targetTransform, AIStrength strength);

  /**
   * @brief Reset movement state
   */
  void reset();

private:
  float m_horizontalTimer = 0.0f;
  float m_currentXDirection = 0.0f;
  float m_idleTimer = 0.0f;
  float m_idleDuration = 0.0f;
  float m_idleCheckTimer = 0.0f; // Timer for checking idle every 3 seconds
  bool m_isIdling = false;

  /**
   * @brief Update horizontal movement direction randomly
   */
  void updateHorizontalDirection();

  /**
   * @brief Calculate vertical velocity to align with target
   */
  float calculateVerticalVelocity(const ecs::Transform &allyTransform, const ecs::Transform &targetTransform, AIStrength strength);

  /**
   * @brief Calculate horizontal velocity
   */
  float calculateHorizontalVelocity(AIStrength strength);

  /**
   * @brief Update idle state for weak AI
   */
  void updateIdleState(float deltaTime, AIStrength strength);

  /**
   * @brief Generate random idle duration
   */
  float generateIdleDuration() const;
};

/**
 * @brief Handles ally shooting decisions
 *
 * Manages:
 * - Shooting timer
 * - Firing condition checks (Y alignment)
 * - Shoot event emission
 * - Charge shot detection for strong AI
 */
class ShootingBehavior
{
public:
  ShootingBehavior();

  /**
   * @brief Update shooting state and emit shoot events if needed
   */
  void update(float deltaTime, ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
              const ecs::Transform &targetTransform, AIStrength strength);

  /**
   * @brief Reset shooting state
   */
  void reset();

private:
  float m_shootingTimer = 0.0f;

  /**
   * @brief Check if ally is aligned vertically with target for shooting
   */
  bool isAlignedForShooting(const ecs::Transform &allyTransform, const ecs::Transform &targetTransform, AIStrength strength);

  /**
   * @brief Emit a shoot event in the direction of target
   */
  void shoot(ecs::World &world, ecs::Entity allyEntity);

  /**
   * @brief Emit a charge shot event (for strong AI with multiple enemies)
   */
  void chargeShoot(ecs::World &world, ecs::Entity allyEntity);

  /**
   * @brief Check if multiple enemies are clustered on the same Y level (for charge shot)
   */
  bool shouldUseChargeShot(ecs::World &world, const ecs::Transform &allyTransform, AIStrength strength);

  /**
   * @brief Count enemies near the same Y level as the target
   */
  int countEnemiesAtYLevel(ecs::World &world, float targetY) const;

  /**
   * @brief Get shooting interval based on AI strength
   */
  float getShootingInterval(AIStrength strength) const;

  /**
   * @brief Determine if the AI should shoot based on strength level
   */
  bool shouldShoot(AIStrength strength) const;
};

/**
 * @brief Handles animation updates based on movement
 *
 * Manages:
 * - Frame selection based on velocity direction
 * - Smooth animation transitions
 */
class AnimationBehavior
{
public:
  /**
   * @brief Update sprite animation frame based on velocity
   */
  static void update(ecs::World &world, ecs::Entity allyEntity, const ecs::Velocity &velocity);

private:
  /**
   * @brief Select animation frame based on vertical velocity
   */
  static int selectAnimationFrame(const ecs::Velocity &velocity);
};

} // namespace server::ai::behavior

#endif // SERVER_ALLY_BEHAVIOR_HPP_
