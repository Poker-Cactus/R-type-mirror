/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyAI.hpp - AI controller for ally entities in solo mode
*/

#ifndef SERVER_ALLY_AI_HPP_
#define SERVER_ALLY_AI_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "AllyBehavior.hpp"
#include "AllyPerception.hpp"

namespace server::ai
{

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
  AllyAI(AIStrength strength = AIStrength::MEDIUM);
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
  // AI strength level
  AIStrength m_strength;

  // Behavior components
  behavior::MovementBehavior m_movement;
  behavior::ShootingBehavior m_shooting;
  behavior::AnimationBehavior m_animation;

  // Perception and response
  perception::ObstacleAvoidance m_avoidance;

  /**
   * @brief Internal update flow
   */
  void updateBehaviors(ecs::World &world, ecs::Entity allyEntity, float deltaTime);
};

} // namespace server::ai

#endif // SERVER_ALLY_AI_HPP_
