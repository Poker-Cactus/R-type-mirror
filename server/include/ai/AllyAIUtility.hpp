/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyAIUtility.hpp - Utility functions and constants for ally AI
*/

#ifndef SERVER_ALLY_AI_UTILITY_HPP_
#define SERVER_ALLY_AI_UTILITY_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include <cmath>

namespace server::ai
{

/**
 * @brief AI strength levels for ally entities
 */
enum class AIStrength { WEAK, MEDIUM, STRONG };

} // namespace server::ai

namespace server::ai::utility
{

// ============================================================================
// CONSTANTS
// ============================================================================

constexpr float ALLY_SPEED = 200.0f;
constexpr float ALLY_SPEED_WEAK_MULTIPLIER = 0.7f; // 70% of normal speed for weak AI
constexpr float ALLY_SPEED_STRONG_MULTIPLIER = 1.2f; // 120% of normal speed for strong AI
constexpr float VERTICAL_ALIGNMENT_THRESHOLD = 50.0f;

// Shooting behavior
constexpr float SHOOTING_INTERVAL = 0.5f; // Shoot every 0.5 seconds
constexpr float SHOOTING_INTERVAL_STRONG_MULTIPLIER = 0.4f; // 40% of interval (faster) for strong AI

// Idle behavior for weak AI
constexpr float IDLE_DURATION_MIN = 2.0f; // Minimum idle time in seconds
constexpr float IDLE_DURATION_MAX = 4.0f; // Maximum idle time in seconds
constexpr float IDLE_CHANCE = 0.3f; // 30% chance to enter idle state

// Charge shot detection for strong AI
constexpr float CHARGE_SHOT_ENEMY_Y_THRESHOLD = 50.0f; // ±50 pixels for enemy clustering
constexpr int CHARGE_SHOT_MIN_ENEMIES = 2; // Minimum enemies for charge shot

// Horizontal movement
constexpr float HORIZONTAL_CHANGE_INTERVAL = 2.0f; // Change direction every 2 seconds
constexpr float HORIZONTAL_SPEED_MULTIPLIER = 0.5f; // Half speed for natural feel

// Obstacle avoidance
constexpr float ENEMY_AVOID_RADIUS = 100.0f;
constexpr float PROJECTILE_AVOID_RADIUS = 100.0f;
constexpr float EMERGENCY_RADIUS = 60.0f;
constexpr float PREDICTION_TIME = 0.5f;
constexpr float AVOID_FORCE_MULTIPLIER = 2.0f;
constexpr float EMERGENCY_MULTIPLIER = 3.0f;
constexpr float HIGH_THREAT_THRESHOLD = 1.5f;

// Center preference for avoiding walls
constexpr float CENTER_PREFERENCE_WEIGHT = 0.3f; // How much to prefer center movement
constexpr float MULTI_THREAT_DEADZONE = 0.2f; // If threats cancel out, move toward center

// Viewport behavior
constexpr float VIEWPORT_USAGE_RATIO = 1.0f / 2.0f; // Ally stays in left 1/3 of screen

// Animation frames
constexpr int ANIMATION_FRAME_UP = 4;
constexpr int ANIMATION_FRAME_DOWN = 0;
constexpr int ANIMATION_FRAME_NEUTRAL = 2;

constexpr float ANIMATION_VELOCITY_THRESHOLD = 10.0f;

// ============================================================================
// MATH UTILITIES
// ============================================================================

/**
 * @brief Calculate Euclidean distance between two points
 */
inline float calculateDistance(float x1, float y1, float x2, float y2)
{
  float dx = x2 - x1;
  float dy = y2 - y1;
  return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Calculate direction vector between two points (normalized)
 * @param x1, y1 Start position
 * @param x2, y2 End position
 * @param outX, outY Output direction components
 * @return Distance between points
 */
inline float calculateDirection(float x1, float y1, float x2, float y2, float &outX, float &outY)
{
  float dx = x2 - x1;
  float dy = y2 - y1;
  float distance = std::sqrt(dx * dx + dy * dy);

  if (distance > 0.0f) {
    outX = dx / distance;
    outY = dy / distance;
  } else {
    outX = 0.0f;
    outY = 0.0f;
  }

  return distance;
}

/**
 * @brief Normalize a vector
 */
inline void normalizeVector(float &x, float &y)
{
  float magnitude = std::sqrt(x * x + y * y);
  if (magnitude > 0.0f) {
    x /= magnitude;
    y /= magnitude;
  }
}

/**
 * @brief Calculate direction toward center of screen
 * @param allyX, allyY Current position
 * @param viewportWidth, viewportHeight Screen dimensions
 * @param outX, outY Output direction components (normalized)
 * @return Distance to center
 */
inline float calculateCenterDirection(float allyX, float allyY, float viewportWidth, float viewportHeight, float &outX,
                                      float &outY)
{
  float centerX = viewportWidth / 2.0f;
  float centerY = viewportHeight / 2.0f;
  return calculateDirection(allyX, allyY, centerX, centerY, outX, outY);
}

/**
 * @brief Get vector magnitude
 */
inline float getVectorMagnitude(float x, float y)
{
  return std::sqrt(x * x + y * y);
}

/**
 * @brief Get collider radius from entity
 * @return Radius, or default value if no collider
 */
float getColliderRadius(ecs::World &world, ecs::Entity entity, float defaultRadius = 20.0f);

/**
 * @brief Get center position of an entity based on its collider
 * @param entity Entity to check
 * @param x Current top-left x
 * @param y Current top-left y
 * @param centerX Output center x
 * @param centerY Output center y
 */
void getEntityCenter(ecs::World &world, ecs::Entity entity, float x, float y, float &centerX, float &centerY);

/**
 * @brief Predict future position of entity based on velocity
 */
void predictEntityPosition(ecs::World &world, ecs::Entity entity, float predictionTime, float &outX, float &outY);

/**
 * @brief Check if entity is alive and has required components
 */
bool isEntityValid(ecs::World &world, ecs::Entity entity);

} // namespace server::ai::utility

#endif // SERVER_ALLY_AI_UTILITY_HPP_
