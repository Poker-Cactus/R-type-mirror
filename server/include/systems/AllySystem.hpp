/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllySystem.hpp - Ally system for controlling ally entities
*/

#ifndef SERVER_ALLY_SYSTEM_HPP_
#define SERVER_ALLY_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/components/Viewport.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <cstdlib>
#include <vector>

namespace server
{

/**
 * @brief System that controls ally entities
 *
 * This system handles the behavior of ally-controlled entities, such as
 * movement decisions, shooting, and other actions based on game state.
 */
class AllySystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    m_shootingTimer += deltaTime;
    m_horizontalMoveTimer += deltaTime;

    // Check if solo mode (only one player)
    std::vector<ecs::Entity> players;
    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    world.getEntitiesWithSignature(playerSig, players);
    bool isSoloMode = (players.size() == 1);

    if (!isSoloMode) {
      return; // Only process allies in solo mode
    }

    std::vector<ecs::Entity> allyEntities;
    world.getEntitiesWithSignature(getSignature(), allyEntities);

    for (auto allyEntity : allyEntities) {
      // Get ally entity's position
      auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
      auto &allyVelocity = world.getComponent<ecs::Velocity>(allyEntity);

      // Find player position
      if (players.empty()) {
        continue; // No player found
      }
      ecs::Entity playerEntity = players[0];
      auto &playerTransform = world.getComponent<ecs::Transform>(playerEntity);

      // Find nearest enemy
      ecs::Entity nearestEnemy = findNearestEnemy(world, allyTransform.x, allyTransform.y, playerEntity);
      if (nearestEnemy != 0) {
        auto &enemyTransform = world.getComponent<ecs::Transform>(nearestEnemy);

        // Ally logic here (to be implemented)
        // For now, just move towards player and shoot at enemies
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, enemyTransform);
        updateAllyShooting(world, allyEntity, allyTransform, enemyTransform);
      } else {
        // No enemies, follow player
        updateAllyMovement(world, allyEntity, allyVelocity, allyTransform, playerTransform);
      }

      // Avoid nearby enemies and projectiles
      avoidObstacles(world, allyEntity, allyVelocity, allyTransform);

      // Update animation based on final movement direction (after avoidance)
      auto &sprite = world.getComponent<ecs::Sprite>(allyEntity);
      if (allyVelocity.dy < -10.0f) { // Moving up
        sprite.currentFrame = 4; // Up animation frame
      } else if (allyVelocity.dy > 10.0f) { // Moving down
        sprite.currentFrame = 0; // Down animation frame
      } else { // Stopped or moving horizontally
        sprite.currentFrame = 2; // Neutral frame
      }

      // Clamp ally position to viewport bounds (same as player)
      if (world.hasComponent<ecs::Viewport>(playerEntity)) {
        const auto &viewport = world.getComponent<ecs::Viewport>(playerEntity);
        if (viewport.width > 0 && viewport.height > 0) {
          // Calculate ally size from collider
          float allyW = 0.0f;
          float allyH = 0.0f;
          if (world.hasComponent<ecs::Collider>(allyEntity)) {
            const auto &col = world.getComponent<ecs::Collider>(allyEntity);
            if (col.shape == ecs::Collider::Shape::BOX) {
              if (col.width > 0.0f)
                allyW = col.width;
              if (col.height > 0.0f)
                allyH = col.height;
            } else {
              // Circle: clamp using diameter so the whole collider stays visible
              const float diameter = col.radius * 2.0f;
              if (diameter > 0.0f) {
                allyW = diameter;
                allyH = diameter;
              }
            }
          }

          float maxX = (static_cast<float>(viewport.width) / 3.0f) - allyW; // Left 1/3 of screen
          float maxY = static_cast<float>(viewport.height) - allyH;
          if (maxX < 0.0f)
            maxX = 0.0f;
          if (maxY < 0.0f)
            maxY = 0.0f;

          if (allyTransform.x < 0.0f)
            allyTransform.x = 0.0f;
          if (allyTransform.x > maxX)
            allyTransform.x = maxX;
          if (allyTransform.y < 0.0f)
            allyTransform.y = 0.0f;
          if (allyTransform.y > maxY)
            allyTransform.y = maxY;
        }
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Ally>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    sig.set(ecs::getComponentId<ecs::Velocity>());
    return sig;
  }

private:
  static constexpr float ALLY_SPEED = 200.0f;

  // Shooting parameters
  float m_shootingTimer = 0.0f;
  static constexpr float SHOOTING_INTERVAL = 0.5f; // Shoot every 0.5 seconds

  // Horizontal movement parameters
  float m_horizontalMoveTimer = 0.0f;
  static constexpr float HORIZONTAL_CHANGE_INTERVAL = 2.0f; // Change X direction every 2 seconds
  float m_currentXDirection = 0.0f; // Current X movement direction (-1, 0, 1)

  /**
   * @brief Find the nearest enemy to the given position that is within screen bounds
   */
  ecs::Entity findNearestEnemy(ecs::World &world, float x, float y, ecs::Entity playerEntity)
  {
    std::vector<ecs::Entity> enemies;
    ecs::ComponentSignature enemySig;
    enemySig.set(ecs::getComponentId<ecs::Transform>());
    enemySig.set(ecs::getComponentId<ecs::Pattern>()); // Enemies have Pattern component
    world.getEntitiesWithSignature(enemySig, enemies);

    // Get viewport bounds from player
    float viewportWidth = 1920.0f;  // Default fallback
    float viewportHeight = 1080.0f; // Default fallback
    if (world.hasComponent<ecs::Viewport>(playerEntity)) {
      const auto &viewport = world.getComponent<ecs::Viewport>(playerEntity);
      viewportWidth = static_cast<float>(viewport.width);
      viewportHeight = static_cast<float>(viewport.height);
    }

    ecs::Entity nearest = 0;
    float minDistance = std::numeric_limits<float>::max();

    for (auto enemy : enemies) {
      if (!world.isAlive(enemy)) continue;
      
      auto &transform = world.getComponent<ecs::Transform>(enemy);
      
      // Skip enemies outside screen bounds
      if (transform.x < 0 || transform.x > viewportWidth ||
          transform.y < 0 || transform.y > viewportHeight) {
        continue;
      }
      
      float dx = transform.x - x;
      float dy = transform.y - y;
      float distance = std::sqrt(dx * dx + dy * dy);

      if (distance < minDistance) {
        minDistance = distance;
        nearest = enemy;
      }
    }

    return nearest;
  }

  void updateAllyMovement(ecs::World &world, ecs::Entity allyEntity, ecs::Velocity &velocity, const ecs::Transform &allyTransform,
                          const ecs::Transform &targetTransform)
  {
    // Handle vertical movement (align with target Y)
    float dy = targetTransform.y - allyTransform.y;
    if (std::abs(dy) > 50.0f) {
      velocity.dy = (dy > 0 ? 1.0f : -1.0f) * ALLY_SPEED;
    } else {
      velocity.dy = 0.0f;
    }

    // Handle horizontal movement (random but natural, constrained to left 1/3)
    if (m_horizontalMoveTimer >= HORIZONTAL_CHANGE_INTERVAL) {
      // Change X direction randomly
      int randomChoice = rand() % 3; // 0, 1, or 2
      if (randomChoice == 0) {
        m_currentXDirection = -1.0f; // Move left
      } else if (randomChoice == 1) {
        m_currentXDirection = 1.0f;  // Move right
      } else {
        m_currentXDirection = 0.0f;  // Stay still
      }
      m_horizontalMoveTimer = 0.0f;
    }

    // Apply X movement at half speed for more natural feel
    velocity.dx = m_currentXDirection * (ALLY_SPEED * 0.5f);

    // Note: Animation update moved to after obstacle avoidance for correct priority
  }

  /**
   * @brief Update ally shooting based on Y alignment with enemy
   */
  void updateAllyShooting(ecs::World &world, ecs::Entity allyEntity, const ecs::Transform &allyTransform,
                          const ecs::Transform &enemyTransform)
  {
    float dy = enemyTransform.y - allyTransform.y;

    if (std::abs(dy) <= 50.0f && m_shootingTimer >= SHOOTING_INTERVAL) {
      // Shoot right (assuming enemies are to the right)
      ecs::ShootEvent shootEvent(allyEntity, 1.0f, 0.0f);
      world.emitEvent(shootEvent);
      m_shootingTimer = 0.0f;
    }
  }

  /**
   * @brief Make ally avoid nearby enemies and projectiles with priority and prediction
   */
  void avoidObstacles(ecs::World &world, ecs::Entity allyEntity, ecs::Velocity &velocity, const ecs::Transform &allyTransform)
  {
    const float ENEMY_AVOID_RADIUS = 180.0f;        // Increased: Avoid enemies within 180 pixels
    const float PROJECTILE_AVOID_RADIUS = 120.0f;   // Increased: Avoid projectiles within 120 pixels
    const float EMERGENCY_RADIUS = 60.0f;           // Emergency avoidance for very close threats
    const float PREDICTION_TIME = 0.5f;             // Predict position 0.5 seconds ahead
    const float AVOID_FORCE_MULTIPLIER = 2.0f;      // Increased: How strongly to avoid obstacles
    const float EMERGENCY_MULTIPLIER = 3.0f;        // Emergency avoidance is much stronger

    float totalAvoidX = 0.0f;
    float totalAvoidY = 0.0f;
    float maxWeight = 0.0f;
    bool needsAvoidance = false;

    // Get ally collider size for better avoidance
    float allyRadius = 20.0f; // Default radius
    if (world.hasComponent<ecs::Collider>(allyEntity)) {
      const auto &col = world.getComponent<ecs::Collider>(allyEntity);
      if (col.shape == ecs::Collider::Shape::BOX) {
        allyRadius = std::max(col.width, col.height) * 0.5f;
      } else {
        allyRadius = col.radius;
      }
    }

    // Check for nearby enemies
    std::vector<ecs::Entity> enemies;
    ecs::ComponentSignature enemySig;
    enemySig.set(ecs::getComponentId<ecs::Transform>());
    enemySig.set(ecs::getComponentId<ecs::Pattern>());
    world.getEntitiesWithSignature(enemySig, enemies);

    for (auto enemy : enemies) {
      if (!world.isAlive(enemy)) continue;
      auto &enemyTransform = world.getComponent<ecs::Transform>(enemy);

      // Calculate predicted position based on velocity
      float predictedX = enemyTransform.x;
      float predictedY = enemyTransform.y;
      if (world.hasComponent<ecs::Velocity>(enemy)) {
        const auto &enemyVel = world.getComponent<ecs::Velocity>(enemy);
        predictedX += enemyVel.dx * PREDICTION_TIME;
        predictedY += enemyVel.dy * PREDICTION_TIME;
      }

      // Get enemy collider size
      float enemyRadius = 20.0f; // Default
      if (world.hasComponent<ecs::Collider>(enemy)) {
        const auto &col = world.getComponent<ecs::Collider>(enemy);
        if (col.shape == ecs::Collider::Shape::BOX) {
          enemyRadius = std::max(col.width, col.height) * 0.5f;
        } else {
          enemyRadius = col.radius;
        }
      }

      float dx = predictedX - allyTransform.x;
      float dy = predictedY - allyTransform.y;
      float distance = std::sqrt(dx * dx + dy * dy);
      float effectiveRadius = allyRadius + enemyRadius;

      if (distance <= ENEMY_AVOID_RADIUS + effectiveRadius && distance > 0.0f) {
        // Calculate avoidance direction (away from predicted enemy position)
        float avoidX = -dx / distance;
        float avoidY = -dy / distance;

        // Weight by proximity and size (stronger when closer or larger)
        float weight = 1.0f - (distance / (ENEMY_AVOID_RADIUS + effectiveRadius));
        weight *= (effectiveRadius / 20.0f); // Larger objects get more weight

        // Emergency avoidance for very close threats
        if (distance <= EMERGENCY_RADIUS + effectiveRadius) {
          weight *= EMERGENCY_MULTIPLIER;
        }

        totalAvoidX += avoidX * weight;
        totalAvoidY += avoidY * weight;
        maxWeight = std::max(maxWeight, weight);
        needsAvoidance = true;
      }
    }

    // Check for nearby projectiles (not owned by ally)
    std::vector<ecs::Entity> projectiles;
    ecs::ComponentSignature projectileSig;
    projectileSig.set(ecs::getComponentId<ecs::Transform>());
    projectileSig.set(ecs::getComponentId<ecs::Owner>());
    world.getEntitiesWithSignature(projectileSig, projectiles);

    for (auto projectile : projectiles) {
      if (!world.isAlive(projectile)) continue;
      auto &projTransform = world.getComponent<ecs::Transform>(projectile);
      auto &owner = world.getComponent<ecs::Owner>(projectile);

      // Skip if owned by this ally
      if (owner.ownerId == allyEntity) continue;

      // Calculate predicted position based on velocity
      float predictedX = projTransform.x;
      float predictedY = projTransform.y;
      if (world.hasComponent<ecs::Velocity>(projectile)) {
        const auto &projVel = world.getComponent<ecs::Velocity>(projectile);
        predictedX += projVel.dx * PREDICTION_TIME;
        predictedY += projVel.dy * PREDICTION_TIME;
      }

      // Get projectile collider size
      float projRadius = 5.0f; // Default smaller for projectiles
      if (world.hasComponent<ecs::Collider>(projectile)) {
        const auto &col = world.getComponent<ecs::Collider>(projectile);
        if (col.shape == ecs::Collider::Shape::BOX) {
          projRadius = std::max(col.width, col.height) * 0.5f;
        } else {
          projRadius = col.radius;
        }
      }

      float dx = predictedX - allyTransform.x;
      float dy = predictedY - allyTransform.y;
      float distance = std::sqrt(dx * dx + dy * dy);
      float effectiveRadius = allyRadius + projRadius;

      if (distance <= PROJECTILE_AVOID_RADIUS + effectiveRadius && distance > 0.0f) {
        // Calculate avoidance direction (away from predicted projectile position)
        float avoidX = -dx / distance;
        float avoidY = -dy / distance;

        // Weight by proximity (projectiles are more urgent)
        float weight = 1.0f - (distance / (PROJECTILE_AVOID_RADIUS + effectiveRadius));
        weight *= 2.0f; // Extra weight for projectiles

        // Emergency avoidance for very close threats
        if (distance <= EMERGENCY_RADIUS + effectiveRadius) {
          weight *= EMERGENCY_MULTIPLIER;
        }

        totalAvoidX += avoidX * weight;
        totalAvoidY += avoidY * weight;
        maxWeight = std::max(maxWeight, weight);
        needsAvoidance = true;
      }
    }

    // If avoidance is needed, override velocity with avoidance movement
    if (needsAvoidance) {
      // Normalize the total avoidance vector
      float avoidMagnitude = std::sqrt(totalAvoidX * totalAvoidX + totalAvoidY * totalAvoidY);
      if (avoidMagnitude > 0.0f) {
        totalAvoidX /= avoidMagnitude;
        totalAvoidY /= avoidMagnitude;

        // Apply avoidance velocity with dynamic speed based on threat level
        float speedMultiplier = AVOID_FORCE_MULTIPLIER;
        if (maxWeight > 1.5f) { // High threat level
          speedMultiplier = EMERGENCY_MULTIPLIER;
        }

        velocity.dx = totalAvoidX * ALLY_SPEED * speedMultiplier;
        velocity.dy = totalAvoidY * ALLY_SPEED * speedMultiplier;
      }
    }
    // If no avoidance needed, normal movement continues unchanged
  }
};

} // namespace server

#endif // SERVER_ALLY_SYSTEM_HPP_
