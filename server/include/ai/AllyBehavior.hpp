/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyBehavior.hpp - Behavior components for ally AI
*/

#ifndef SERVER_ALLY_BEHAVIOR_HPP_
#define SERVER_ALLY_BEHAVIOR_HPP_

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
 */
class MovementBehavior
{
public:
  MovementBehavior();

  /**
   * @brief Update movement velocities based on target position
   */
  void update(float deltaTime, ecs::Velocity &allyVelocity, const ecs::Transform &allyTransform,
              const ecs::Transform &targetTransform);

  /**
   * @brief Reset movement state
   */
  void reset();

private:
  float m_horizontalTimer = 0.0f;
  float m_currentXDirection = 0.0f;

  /**
   * @brief Update horizontal movement direction randomly
   */
  void updateHorizontalDirection();

  /**
   * @brief Calculate vertical velocity to align with target
   */
  float calculateVerticalVelocity(const ecs::Transform &allyTransform, const ecs::Transform &targetTransform);

  /**
   * @brief Calculate horizontal velocity
   */
  float calculateHorizontalVelocity();
};

/**
 * @brief Handles ally shooting decisions
 *
 * Manages:
 * - Shooting timer
 * - Firing condition checks (Y alignment)
 * - Shoot event emission
 */
class ShootingBehavior
{
public:
  ShootingBehavior();

  /**
   * @brief Update shooting state and emit shoot events if needed
   */
  void update(float deltaTime, ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
              const ecs::Transform &targetTransform);

  /**
   * @brief Reset shooting state
   */
  void reset();

private:
  float m_shootingTimer = 0.0f;

  /**
   * @brief Check if ally is aligned vertically with target for shooting
   */
  bool isAlignedForShooting(const ecs::Transform &allyTransform, const ecs::Transform &targetTransform);

  /**
   * @brief Emit a shoot event in the direction of target
   */
  void shoot(ecs::World &world, ecs::Entity allyEntity);
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
