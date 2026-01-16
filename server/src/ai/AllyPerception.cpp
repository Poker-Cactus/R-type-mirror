/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllyPerception.cpp - Implementation of perception and avoidance systems
*/

#include "../../include/ai/AllyPerception.hpp"
#include "../../include/ai/AllyAIUtility.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Viewport.hpp"
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace server::ai::perception
{

// ============================================================================
// EnemyPerception
// ============================================================================

ecs::Entity EnemyPerception::findNearestEnemy(ecs::World &world, float allyX, float allyY, ecs::Entity playerEntity)
{
  // Gather all enemies (entities with Pattern component)
  std::vector<ecs::Entity> enemies;
  ecs::ComponentSignature enemySig;
  enemySig.set(ecs::getComponentId<ecs::Transform>());
  enemySig.set(ecs::getComponentId<ecs::Pattern>());
  world.getEntitiesWithSignature(enemySig, enemies);

  ecs::Entity nearest = 0;
  float minDistance = std::numeric_limits<float>::max();

  for (auto enemy : enemies) {
    // Skip dead or invalid enemies
    if (!world.isAlive(enemy) || !world.hasComponent<ecs::Transform>(enemy)) {
      continue;
    }

    auto &transform = world.getComponent<ecs::Transform>(enemy);

    // Skip enemies outside viewport
    if (!isWithinViewportBounds(transform.x, transform.y, world, playerEntity)) {
      continue;
    }

    // Calculate distance
    float distance = utility::calculateDistance(allyX, allyY, transform.x, transform.y);

    if (distance < minDistance) {
      minDistance = distance;
      nearest = enemy;
    }
  }

  return nearest;
}

bool EnemyPerception::isWithinViewportBounds(float x, float y, const ecs::World &world, ecs::Entity playerEntity)
{
  float width, height;
  getViewportBounds(world, playerEntity, width, height);

  return (x >= 0.0f && x <= width && y >= 0.0f && y <= height);
}

void EnemyPerception::getViewportBounds(const ecs::World &world, ecs::Entity playerEntity, float &outWidth,
                                        float &outHeight)
{
  // Default fallback
  outWidth = 1920.0f;
  outHeight = 1080.0f;

  if (!world.hasComponent<ecs::Viewport>(playerEntity)) {
    return;
  }

  const auto &viewport = world.getComponent<ecs::Viewport>(playerEntity);
  outWidth = static_cast<float>(viewport.width);
  outHeight = static_cast<float>(viewport.height);
}

// ============================================================================
// ObstacleAvoidance
// ============================================================================

ObstacleAvoidance::ObstacleAvoidance()
{
}

void ObstacleAvoidance::update(ecs::World &world, ecs::Entity allyEntity, ecs::Velocity &allyVelocity,
                               const ecs::Transform &allyTransform)
{
  AvoidanceState state;

  // Get ally size
  float allyRadius = utility::getColliderRadius(world, allyEntity);

  // Evaluate threats from enemies and projectiles
  evaluateEnemyThreats(world, allyEntity, allyTransform, allyRadius, state);
  evaluateProjectileThreats(world, allyEntity, allyTransform, allyRadius, state);

  // Apply avoidance if threats detected
  if (state.needsAvoidance) {
    // Get viewport dimensions for center preference calculation
    float viewportWidth = 1920.0f;
    float viewportHeight = 1080.0f;
    if (world.hasComponent<ecs::Viewport>(allyEntity)) {
      const auto &viewport = world.getComponent<ecs::Viewport>(allyEntity);
      viewportWidth = static_cast<float>(viewport.width);
      viewportHeight = static_cast<float>(viewport.height);
    }

    applyAvoidanceForce(allyVelocity, state, allyTransform, viewportWidth, viewportHeight);
  }
}

void ObstacleAvoidance::reset()
{
}

void ObstacleAvoidance::evaluateEnemyThreats(ecs::World &world, ecs::Entity /*allyEntity*/,
                                             const ecs::Transform &allyTransform, float allyRadius,
                                             AvoidanceState &state)
{
  // Get all enemies
  std::vector<ecs::Entity> enemies;
  ecs::ComponentSignature enemySig;
  enemySig.set(ecs::getComponentId<ecs::Transform>());
  enemySig.set(ecs::getComponentId<ecs::Pattern>());
  world.getEntitiesWithSignature(enemySig, enemies);

  for (auto enemy : enemies) {
    if (!world.isAlive(enemy) || !world.hasComponent<ecs::Transform>(enemy)) {
      continue;
    }

    // Get enemy transform for prediction calculation
    auto &enemyTransformRef = world.getComponent<ecs::Transform>(enemy);

    // Predict enemy position (based on current position and velocity)
    float predictedX = enemyTransformRef.x;
    float predictedY = enemyTransformRef.y;
    if (world.hasComponent<ecs::Velocity>(enemy)) {
      const auto &enemyVel = world.getComponent<ecs::Velocity>(enemy);
      predictedX += enemyVel.dx * utility::PREDICTION_TIME;
      predictedY += enemyVel.dy * utility::PREDICTION_TIME;
    }

    // Get enemy radius
    float enemyRadius = utility::getColliderRadius(world, enemy);

    // Calculate threat
    float dirX, dirY, distance;
    calculateAvoidanceDirection(predictedX, predictedY, allyTransform, dirX, dirY, distance);

    float effectiveRadius = allyRadius + enemyRadius;

    // Check if threat is within avoidance radius
    if (distance <= utility::ENEMY_AVOID_RADIUS + effectiveRadius && distance > 0.0f) {
      bool isEmergency = distance <= utility::EMERGENCY_RADIUS + effectiveRadius;
      float weight = calculateThreatWeight(distance, utility::ENEMY_AVOID_RADIUS, effectiveRadius, false, isEmergency);

      state.totalX += dirX * weight;
      state.totalY += dirY * weight;
      state.maxWeight = std::max(state.maxWeight, weight);
      state.threatCount++; // Track multiple threats
      state.needsAvoidance = true;
    }
  }
}

void ObstacleAvoidance::evaluateProjectileThreats(ecs::World &world, ecs::Entity allyEntity,
                                                  const ecs::Transform &allyTransform, float allyRadius,
                                                  AvoidanceState &state)
{
  // Get all projectiles
  std::vector<ecs::Entity> projectiles;
  ecs::ComponentSignature projectileSig;
  projectileSig.set(ecs::getComponentId<ecs::Transform>());
  projectileSig.set(ecs::getComponentId<ecs::Owner>());
  world.getEntitiesWithSignature(projectileSig, projectiles);

  for (auto projectile : projectiles) {
    if (!world.isAlive(projectile) || !world.hasComponent<ecs::Transform>(projectile) ||
        !world.hasComponent<ecs::Owner>(projectile)) {
      continue;
    }

    auto &owner = world.getComponent<ecs::Owner>(projectile);

    // Skip projectiles owned by this ally
    if (owner.ownerId == allyEntity) {
      continue;
    }

    auto &projTransform = world.getComponent<ecs::Transform>(projectile);

    // Predict projectile position (based on current position and velocity)
    float predictedX = projTransform.x;
    float predictedY = projTransform.y;
    if (world.hasComponent<ecs::Velocity>(projectile)) {
      const auto &projVel = world.getComponent<ecs::Velocity>(projectile);
      predictedX += projVel.dx * utility::PREDICTION_TIME;
      predictedY += projVel.dy * utility::PREDICTION_TIME;
    }

    // Get projectile radius
    float projRadius = utility::getColliderRadius(world, projectile, 5.0f);

    // Calculate threat
    float dirX, dirY, distance;
    calculateAvoidanceDirection(predictedX, predictedY, allyTransform, dirX, dirY, distance);

    float effectiveRadius = allyRadius + projRadius;

    // Check if threat is within avoidance radius
    if (distance <= utility::PROJECTILE_AVOID_RADIUS + effectiveRadius && distance > 0.0f) {
      bool isEmergency = distance <= utility::EMERGENCY_RADIUS + effectiveRadius;
      float weight = calculateThreatWeight(distance, utility::PROJECTILE_AVOID_RADIUS, effectiveRadius, true, isEmergency);

      state.totalX += dirX * weight;
      state.totalY += dirY * weight;
      state.maxWeight = std::max(state.maxWeight, weight);
      state.threatCount++; // Track multiple threats
      state.needsAvoidance = true;
    }
  }
}

void ObstacleAvoidance::applyAvoidanceForce(ecs::Velocity &allyVelocity, const AvoidanceState &state,
                                            const ecs::Transform &allyTransform, float viewportWidth,
                                            float viewportHeight)
{
  // Normalize avoidance vector (create mutable copies)
  float totalX = state.totalX;
  float totalY = state.totalY;
  utility::normalizeVector(totalX, totalY);

  // Apply center preference to avoid edge-pinning with multiple threats
  applyCenterPreference(totalX, totalY, allyTransform.x, allyTransform.y, viewportWidth, viewportHeight,
                        state.threatCount);

  // Determine speed multiplier based on threat level
  float speedMultiplier = utility::AVOID_FORCE_MULTIPLIER;
  if (state.maxWeight > utility::HIGH_THREAT_THRESHOLD) {
    speedMultiplier = utility::EMERGENCY_MULTIPLIER;
  }

  // Apply avoidance velocity
  allyVelocity.dx = totalX * utility::ALLY_SPEED * speedMultiplier;
  allyVelocity.dy = totalY * utility::ALLY_SPEED * speedMultiplier;
}

float ObstacleAvoidance::calculateThreatWeight(float distance, float avoidanceRadius, float effectiveRadius,
                                               bool isProjectile, bool isEmergency)
{
  // Base weight from distance
  float weight = 1.0f - (distance / (avoidanceRadius + effectiveRadius));

  // Scale by object size
  weight *= (effectiveRadius / 20.0f);

  // Projectiles are more urgent
  if (isProjectile) {
    weight *= 2.0f;
  }

  // Emergency multiplier for very close threats
  if (isEmergency) {
    weight *= utility::EMERGENCY_MULTIPLIER;
  }

  return weight;
}

void ObstacleAvoidance::calculateAvoidanceDirection(float threatX, float threatY, const ecs::Transform &allyTransform,
                                                    float &outDirX, float &outDirY, float &outDistance)
{
  // Direction away from threat
  outDistance = utility::calculateDirection(threatX, threatY, allyTransform.x, allyTransform.y, outDirX, outDirY);
}

// ============================================================================
// ViewportConstraint
// ============================================================================

void ViewportConstraint::constrainToViewport(ecs::World &world, ecs::Entity allyEntity, ecs::Entity playerEntity)
{
  if (!world.hasComponent<ecs::Transform>(allyEntity) || !world.hasComponent<ecs::Viewport>(playerEntity)) {
    return;
  }

  auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
  const auto &viewport = world.getComponent<ecs::Viewport>(playerEntity);

  if (viewport.width <= 0 || viewport.height <= 0) {
    return;
  }

  // Get ally dimensions
  float allyWidth, allyHeight;
  getAllySize(world, allyEntity, allyWidth, allyHeight);

  // Calculate maximum bounds (left 1/3 of screen)
  float maxX, maxY;
  calculateMaxBounds(static_cast<float>(viewport.width), static_cast<float>(viewport.height), allyWidth, allyHeight,
                     maxX, maxY);

  // Clamp position
  allyTransform.x = std::max(0.0f, std::min(allyTransform.x, maxX));
  allyTransform.y = std::max(0.0f, std::min(allyTransform.y, maxY));
}

void ViewportConstraint::getAllySize(ecs::World &world, ecs::Entity allyEntity, float &outWidth, float &outHeight)
{
  outWidth = 0.0f;
  outHeight = 0.0f;

  if (!world.hasComponent<ecs::Collider>(allyEntity)) {
    return;
  }

  const auto &col = world.getComponent<ecs::Collider>(allyEntity);

  if (col.shape == ecs::Collider::Shape::BOX) {
    outWidth = (col.width > 0.0f) ? col.width : 0.0f;
    outHeight = (col.height > 0.0f) ? col.height : 0.0f;
  } else {
    // Circle: use diameter
    float diameter = col.radius * 2.0f;
    if (diameter > 0.0f) {
      outWidth = diameter;
      outHeight = diameter;
    }
  }
}

void ViewportConstraint::calculateMaxBounds(float viewportWidth, float viewportHeight, float allyWidth,
                                            float allyHeight, float &outMaxX, float &outMaxY)
{
  // Ally stays within left 1/3 of screen
  outMaxX = (viewportWidth * utility::VIEWPORT_USAGE_RATIO) - allyWidth;
  outMaxY = viewportHeight - allyHeight;

  // Ensure max values don't go negative
  if (outMaxX < 0.0f)
    outMaxX = 0.0f;
  if (outMaxY < 0.0f)
    outMaxY = 0.0f;
}

void ObstacleAvoidance::applyCenterPreference(float &avoidX, float &avoidY, float allyX, float allyY,
                                              float viewportWidth, float viewportHeight, int threatCount)
{
  // Check if avoidance vector is too weak (multiple threats canceling out)
  float avoidMagnitude = utility::getVectorMagnitude(avoidX, avoidY);

  // If threats cancel out (deadzone) or multiple threats detected, prefer center movement
  if (avoidMagnitude < utility::MULTI_THREAT_DEADZONE || threatCount > 1) {
    // Calculate direction toward center
    float centerDirX, centerDirY;
    utility::calculateCenterDirection(allyX, allyY, viewportWidth, viewportHeight, centerDirX, centerDirY);

    // Blend center preference with avoidance direction
    float weight = utility::CENTER_PREFERENCE_WEIGHT;

    // If avoidance is weak, heavily favor center
    if (avoidMagnitude < utility::MULTI_THREAT_DEADZONE) {
      weight = 0.8f; // Mostly center movement
    }
    // If multiple threats, moderately favor center to avoid pinning
    else if (threatCount > 2) {
      weight = 0.5f; // Balance avoidance and center
    }

    // Blend the vectors
    avoidX = avoidX * (1.0f - weight) + centerDirX * weight;
    avoidY = avoidY * (1.0f - weight) + centerDirY * weight;

    // Renormalize after blending
    utility::normalizeVector(avoidX, avoidY);
  }
}

} // namespace server::ai::perception
