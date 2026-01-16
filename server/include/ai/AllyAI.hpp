/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyAI.hpp - AI controller for ally entities in solo mode
*/

#ifndef SERVER_ALLY_AI_HPP_
#define SERVER_ALLY_AI_HPP_

#include "AllyBehavior.hpp"
#include "AllyPerception.hpp"
#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/World.hpp"

namespace server::ai
{

/**
 * @brief AI difficulty levels for ally entities
 */
enum class AIDifficulty
{
  WEAK,    // Slower, less accurate, random idle periods
  MEDIUM,  // Current AI behavior (balanced)
  STRONG   // Faster, more accurate, better avoidance
};

/**
 * @brief Configuration for different AI difficulty levels
 */
struct AIConfig
{
  // Movement parameters
  float speedMultiplier;        // Multiplier for base ally speed
  float horizontalSpeedMultiplier; // Multiplier for horizontal wandering

  // Shooting parameters
  float shootingInterval;       // Time between shots (seconds)
  float shootingAccuracy;       // How well the AI aims (0.0 = random, 1.0 = perfect)

  // Avoidance parameters
  float avoidanceRadiusMultiplier; // Multiplier for threat detection radius
  float emergencyMultiplier;    // Emergency speed boost

  // Special behaviors for weak AI
  float idleCooldownMin;        // Minimum idle time (seconds)
  float idleCooldownMax;        // Maximum idle time (seconds)
  float idleProbability;        // Chance to enter idle state (0.0-1.0)

  // Constructor with defaults
  AIConfig(float speedMult = 1.0f, float horizMult = 1.0f, float shootInt = 0.5f,
           float accuracy = 1.0f, float avoidMult = 1.0f, float emergMult = 3.0f,
           float idleMin = 0.0f, float idleMax = 0.0f, float idleProb = 0.0f)
    : speedMultiplier(speedMult), horizontalSpeedMultiplier(horizMult),
      shootingInterval(shootInt), shootingAccuracy(accuracy),
      avoidanceRadiusMultiplier(avoidMult), emergencyMultiplier(emergMult),
      idleCooldownMin(idleMin), idleCooldownMax(idleMax), idleProbability(idleProb) {}
};

/**
 * @brief Main AI controller for ally entities in solo mode
 *
 * Orchestrates multiple behavior systems:
 * - Perception: Enemy detection, obstacle awareness
 * - Behavior: Movement, shooting, animation
 * - Constraints: Viewport boundaries
 *
 * The AI prioritizes:
 * 1. Emergency avoidance (immediate threats)
 * 2. Enemy pursuit and engagement
 * 3. Following player when no enemies
 * 4. Natural movement patterns
 */
class AllyAI
{
public:
  // Static configurations for each difficulty level
  static const AIConfig WEAK_CONFIG;
  static const AIConfig MEDIUM_CONFIG;
  static const AIConfig STRONG_CONFIG;

  AllyAI(AIDifficulty difficulty = AIDifficulty::MEDIUM);
  ~AllyAI() = default;

  /**
   * @brief Update the ally AI for one frame
   *
   * This orchestrates all AI behaviors:
   * 1. Perceive nearest enemy
   * 2. Calculate movement toward target
   * 3. Evaluate and avoid obstacles
   * 4. Update shooting behavior
   * 5. Update animation
   * 6. Apply viewport constraints
   *
   * @param world The ECS world
   * @param allyEntity The entity controlled by this AI
   * @param deltaTime The time step in seconds
   */
  void update(ecs::World &world, ecs::Entity allyEntity, float deltaTime);

  /**
   * @brief Reset AI state (for reuse)
   */
  void reset();

private:
  // AI configuration
  AIDifficulty m_difficulty;
  AIConfig m_config;

  // Behavior components
  behavior::MovementBehavior m_movement;
  behavior::ShootingBehavior m_shooting;
  behavior::AnimationBehavior m_animation;

  // Perception and response
  perception::ObstacleAvoidance m_avoidance;

  // Special state for weak AI
  float m_idleCooldown;         // Time remaining in idle state
  bool m_isIdle;                // Whether AI is currently idle

  /**
   * @brief Internal update flow
   */
  void updateBehaviors(ecs::World &world, ecs::Entity allyEntity, float deltaTime);

  /**
   * @brief Handle idle behavior for weak AI
   */
  void updateIdleBehavior(float deltaTime);

  /**
   * @brief Get configuration for a difficulty level
   */
  static const AIConfig& getConfigForDifficulty(AIDifficulty difficulty);
};

} // namespace server::ai

#endif // SERVER_ALLY_AI_HPP_
