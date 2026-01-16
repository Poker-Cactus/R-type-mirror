/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyPerception.hpp - Perception and avoidance for ally AI
*/

#ifndef SERVER_ALLY_PERCEPTION_HPP_
#define SERVER_ALLY_PERCEPTION_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"

namespace server::ai::perception
{

// Avoidance weights and behavior
struct AvoidanceState
{
  float totalX = 0.0f;
  float totalY = 0.0f;
  float maxWeight = 0.0f;
  bool needsAvoidance = false;
  int threatCount = 0; // Track number of threats
};

/**
 * @brief Handles enemy detection and tracking
 *
 * Manages:
 * - Finding nearest enemy on screen
 * - Filtering out-of-bounds enemies
 * - Distance calculations
 */
class EnemyPerception
{
public:
  /**
   * @brief Find the nearest visible enemy
   * @return Entity ID of nearest enemy, 0 if none found
   */
  static ecs::Entity findNearestEnemy(ecs::World &world, float allyX, float allyY, ecs::Entity playerEntity);

private:
  /**
   * @brief Check if position is within viewport bounds
   */
  static bool isWithinViewportBounds(float x, float y, const ecs::World &world, ecs::Entity playerEntity);

  /**
   * @brief Get viewport dimensions
   */
  static void getViewportBounds(const ecs::World &world, ecs::Entity playerEntity, float &outWidth, float &outHeight);
};

/**
 * @brief Handles obstacle avoidance with intelligent prioritization
 *
 * Manages:
 * - Predictive avoidance (predicts future positions)
 * - Priority-based collision avoidance (enemies > projectiles)
 * - Emergency response for immediate threats
 * - Velocity override with smooth transitions
 */
class ObstacleAvoidance
{
public:
  ObstacleAvoidance();

  /**
   * @brief Update avoidance velocity based on nearby threats
   */
  void update(ecs::World &world, ecs::Entity allyEntity, ecs::Velocity &allyVelocity,
              const ecs::Transform &allyTransform);

  /**
   * @brief Reset avoidance state
   */
  void reset();

private:
  /**
   * @brief Evaluate threat from enemies
   */
  void evaluateEnemyThreats(ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
                            float allyRadius, AvoidanceState &state);

  /**
   * @brief Evaluate threat from projectiles
   */
  void evaluateProjectileThreats(ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
                                 float allyRadius, AvoidanceState &state);

  /**
   * @brief Apply calculated avoidance to velocity
   */
  void applyAvoidanceForce(ecs::Velocity &allyVelocity, const AvoidanceState &state,
                          const ecs::Transform &allyTransform, float viewportWidth, float viewportHeight);

  /**
   * @brief Calculate weight for obstacle based on distance and threat level
   */
  float calculateThreatWeight(float distance, float avoidanceRadius, float effectiveRadius, bool isProjectile,
                              bool isEmergency);

  /**
   * @brief Calculate avoidance direction away from threat
   */
  void calculateAvoidanceDirection(float threatX, float threatY, const ecs::Transform &allyTransform,
                                   float &outDirX, float &outDirY, float &outDistance);

  /**
   * @brief Apply center preference to avoidance vector
   * Blends avoidance direction with center movement to prevent edge-pinning
   */
  void applyCenterPreference(float &avoidX, float &avoidY, float allyX, float allyY,
                            float viewportWidth, float viewportHeight, int threatCount);
};

/**
 * @brief Handles viewport boundary constraints
 *
 * Manages:
 * - Keeping ally within screen bounds
 * - Using viewport dimensions
 * - Collider-aware boundary clamping
 */
class ViewportConstraint
{
public:
  /**
   * @brief Clamp ally position to viewport bounds (left 1/3 of screen)
   */
  static void constrainToViewport(ecs::World &world, ecs::Entity allyEntity, ecs::Entity playerEntity);

private:
  /**
   * @brief Get ally's collider dimensions
   */
  static void getAllySize(ecs::World &world, ecs::Entity allyEntity, float &outWidth, float &outHeight);

  /**
   * @brief Calculate maximum position within constraints
   */
  static void calculateMaxBounds(float viewportWidth, float viewportHeight, float allyWidth, float allyHeight,
                                 float &outMaxX, float &outMaxY);
};

} // namespace server::ai::perception

#endif // SERVER_ALLY_PERCEPTION_HPP_
