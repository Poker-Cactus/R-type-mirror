/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyAI.cpp - Implementation of main ally AI controller
*/

#include "../../include/ai/AllyAI.hpp"
#include "../../include/ai/AllyAIUtility.hpp"
#include "../../include/ai/AllyBehavior.hpp"
#include "../../include/ai/AllyPerception.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/ComponentSignature.hpp"

namespace server::ai
{

// Static configuration definitions
const AIConfig AllyAI::WEAK_CONFIG(
  0.6f,   // speedMultiplier - 60% of normal speed
  0.4f,   // horizontalSpeedMultiplier - 40% horizontal speed
  1.2f,   // shootingInterval - shoots every 1.2 seconds (slower)
  0.6f,   // shootingAccuracy - 60% accuracy (more random aiming)
  0.8f,   // avoidanceRadiusMultiplier - 80% avoidance radius (less aware)
  2.0f,   // emergencyMultiplier - 2x emergency speed (less responsive)
  1.0f,   // idleCooldownMin - minimum 1 second idle
  3.0f,   // idleCooldownMax - maximum 3 seconds idle
  0.3f    // idleProbability - 30% chance to enter idle state
);

const AIConfig AllyAI::MEDIUM_CONFIG(
  1.0f,   // speedMultiplier - normal speed
  1.0f,   // horizontalSpeedMultiplier - normal horizontal speed
  0.5f,   // shootingInterval - shoots every 0.5 seconds
  1.0f,   // shootingAccuracy - perfect accuracy
  1.0f,   // avoidanceRadiusMultiplier - normal avoidance radius
  3.0f,   // emergencyMultiplier - 3x emergency speed
  0.0f,   // idleCooldownMin - no idle
  0.0f,   // idleCooldownMax - no idle
  0.0f    // idleProbability - no idle
);

const AIConfig AllyAI::STRONG_CONFIG(
  1.4f,   // speedMultiplier - 140% of normal speed
  1.2f,   // horizontalSpeedMultiplier - 120% horizontal speed
  0.35f,  // shootingInterval - shoots every 0.35 seconds (faster)
  1.0f,   // shootingAccuracy - perfect accuracy
  1.3f,   // avoidanceRadiusMultiplier - 130% avoidance radius (more aware)
  4.0f,   // emergencyMultiplier - 4x emergency speed (very responsive)
  0.0f,   // idleCooldownMin - no idle
  0.0f,   // idleCooldownMax - no idle
  0.0f    // idleProbability - no idle
);

AllyAI::AllyAI(AIDifficulty difficulty)
  : m_difficulty(difficulty),
    m_config(getConfigForDifficulty(difficulty)),
    m_idleCooldown(0.0f),
    m_isIdle(false)
{
}

const AIConfig& AllyAI::getConfigForDifficulty(AIDifficulty difficulty)
{
  switch (difficulty) {
    case AIDifficulty::WEAK: return WEAK_CONFIG;
    case AIDifficulty::STRONG: return STRONG_CONFIG;
    case AIDifficulty::MEDIUM:
    default: return MEDIUM_CONFIG;
  }
}

void AllyAI::update(ecs::World &world, ecs::Entity allyEntity, float deltaTime)
{
  // Validate entity
  if (!world.isAlive(allyEntity) || !utility::isEntityValid(world, allyEntity)) {
    return;
  }

  // Only process in solo mode (single player)
  std::vector<ecs::Entity> players;
  ecs::ComponentSignature playerSig;
  playerSig.set(ecs::getComponentId<ecs::PlayerId>());
  world.getEntitiesWithSignature(playerSig, players);

  if (players.size() != 1) {
    return; // Not solo mode
  }

  ecs::Entity playerEntity = players[0];

  // Perform all AI updates
  updateBehaviors(world, allyEntity, deltaTime);

  // Apply viewport constraints at the end
  perception::ViewportConstraint::constrainToViewport(world, allyEntity, playerEntity);
}

void AllyAI::reset()
{
  m_movement.reset();
  m_shooting.reset();
  m_avoidance.reset();
}

void AllyAI::updateBehaviors(ecs::World &world, ecs::Entity allyEntity, float deltaTime)
{
  // Handle idle behavior for weak AI
  updateIdleBehavior(deltaTime);
  
  // If AI is idle, skip all other behaviors
  if (m_isIdle) {
    // Still apply minimal animation and constraints
    auto &allyVelocity = world.getComponent<ecs::Velocity>(allyEntity);
    allyVelocity.dx = 0.0f;
    allyVelocity.dy = 0.0f;
    m_animation.update(world, allyEntity, allyVelocity);
    return;
  }

  // Get ally's current state
  auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
  auto &allyVelocity = world.getComponent<ecs::Velocity>(allyEntity);

  // Find player for reference
  std::vector<ecs::Entity> players;
  ecs::ComponentSignature playerSig;
  playerSig.set(ecs::getComponentId<ecs::PlayerId>());
  world.getEntitiesWithSignature(playerSig, players);

  if (players.empty()) {
    return;
  }

  ecs::Entity playerEntity = players[0];
  auto &playerTransform = world.getComponent<ecs::Transform>(playerEntity);

  // STEP 1: Detect nearest enemy
  ecs::Entity targetEntity = perception::EnemyPerception::findNearestEnemy(world, allyTransform.x, allyTransform.y, playerEntity);

  // Use enemy as target, or player if no enemy
  const ecs::Transform &targetTransform = (targetEntity != 0) ? world.getComponent<ecs::Transform>(targetEntity)
                                                               : playerTransform;

  // STEP 2: Update movement toward target (with difficulty-based speed)
  m_movement.update(deltaTime, allyVelocity, allyTransform, targetTransform, m_config.speedMultiplier, m_config.horizontalSpeedMultiplier);

  // STEP 3: Update shooting (only if enemy detected, with difficulty-based parameters)
  if (targetEntity != 0) {
    m_shooting.update(deltaTime, world, allyEntity, allyTransform, targetTransform, m_config.shootingInterval, m_config.shootingAccuracy);
  }

  // STEP 4: Apply obstacle avoidance (highest priority, with difficulty-based parameters)
  m_avoidance.update(world, allyEntity, allyVelocity, allyTransform, m_config.avoidanceRadiusMultiplier, m_config.emergencyMultiplier);

  // STEP 5: Update animation based on final velocity
  m_animation.update(world, allyEntity, allyVelocity);
}

void AllyAI::updateIdleBehavior(float deltaTime)
{
  // Only weak AI has idle behavior
  if (m_difficulty != AIDifficulty::WEAK) {
    return;
  }

  // Update idle cooldown
  if (m_idleCooldown > 0.0f) {
    m_idleCooldown -= deltaTime;
    if (m_idleCooldown <= 0.0f) {
      m_isIdle = false;
      m_idleCooldown = 0.0f;
    }
    return;
  }

  // Not idle, check if we should enter idle state
  if (!m_isIdle && m_config.idleProbability > 0.0f) {
    // Generate random number between 0 and 1
    float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    
    if (randomValue < m_config.idleProbability) {
      // Enter idle state
      m_isIdle = true;
      // Set random cooldown between min and max
      float range = m_config.idleCooldownMax - m_config.idleCooldownMin;
      m_idleCooldown = m_config.idleCooldownMin + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * range;
    }
  }
}

} // namespace server::ai
