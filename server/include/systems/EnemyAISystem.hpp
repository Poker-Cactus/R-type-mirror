/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EnemyAISystem.hpp - Enemy AI behavior
*/

#ifndef SERVER_ENEMY_AI_SYSTEM_HPP_
#define SERVER_ENEMY_AI_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
#include <random>
#include <vector>

namespace server
{

/**
 * @brief System that controls enemy AI behavior
 */
class EnemyAISystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      auto &transform = world.getComponent<ecs::Transform>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);
      auto &pattern = world.getComponent<ecs::Pattern>(entity);

      // Apply pattern-based movement
      if (pattern.patternType == "sine_wave") {
        // Horizontal movement (always moving left)
        velocity.dx = ENEMY_MOVE_SPEED;

        // Update phase based on deltaTime and frequency
        pattern.phase += deltaTime * pattern.frequency;

        // Calculate vertical velocity using sine wave
        // velocity = amplitude * frequency * cos(phase)
        velocity.dy = pattern.amplitude * pattern.frequency * std::cos(pattern.phase);

      } else if (pattern.patternType == "zigzag") {
        // Zigzag diagonal movement (for bee enemies)
        // Zigzag: diagonal up, then diagonal down in a repeating pattern
        // amplitude = hauteur totale du zigzag
        // frequency = vitesse de changement de direction

        // Calculer la position Y relative au point de spawn
        float relativeY = transform.y - pattern.phase; // phase stocke le Y initial

        // Si on n'a pas encore initialisé le Y de départ
        if (pattern.phase == 0.0F) {
          pattern.phase = transform.y; // Stocker le Y initial dans phase
          relativeY = 0.0F;
        }

        // Changer de direction quand on atteint l'amplitude
        if (relativeY > pattern.amplitude) {
          velocity.dy = -std::abs(velocity.dy); // Vers le bas
        } else if (relativeY < -pattern.amplitude) {
          velocity.dy = std::abs(velocity.dy); // Vers le haut
        }

        // Garder la vélocité diagonale constante
        // velocity.dx reste inchangé (mouvement horizontal)
        // velocity.dy est déjà défini dans la config et inversé selon la position

      } else if (pattern.patternType == "straight") {
        // Simple straight-line movement
        velocity.dx = ENEMY_MOVE_SPEED;
        velocity.dy = 0.0F;
      } else if (pattern.patternType == "bounce") {
        // Bounce pattern: diagonal movement that bounces off screen boundaries
        // The robot moves diagonally and bounces when hitting top, bottom, or sides
        
        constexpr float SCREEN_TOP_BOUNDARY = 0.0F;
        constexpr float SCREEN_BOTTOM_BOUNDARY = 1080.0F;
        constexpr float SCREEN_LEFT_BOUNDARY = 0.0F;
        constexpr float SCREEN_RIGHT_BOUNDARY = 1920.0F;
        
        // Initialize random direction on first frame (use phase to track if initialized)
        if (pattern.phase == 0.0F) {
          // Mark as initialized by setting phase to 1.0
          pattern.phase = 1.0F;
          
          // Randomize initial vertical direction (50% up, 50% down)
          // Use entity ID + position as seed for truly varied behavior per individual
          std::mt19937 rng(static_cast<unsigned int>(entity) + static_cast<unsigned int>(transform.x * 100.0F) + static_cast<unsigned int>(transform.y * 100.0F));
          std::uniform_int_distribution<int> dist(0, 1);
          
          if (dist(rng) == 0) {
            velocity.dy = std::abs(velocity.dy); // Go up
          } else {
            velocity.dy = -std::abs(velocity.dy); // Go down
          }
        }
        
        // Check boundaries and reverse direction if needed
        // Top boundary
        if (transform.y <= SCREEN_TOP_BOUNDARY && velocity.dy < 0.0F) {
          velocity.dy = -velocity.dy; // Bounce down
        }
        // Bottom boundary
        if (transform.y >= SCREEN_BOTTOM_BOUNDARY && velocity.dy > 0.0F) {
          velocity.dy = -velocity.dy; // Bounce up
        }
        // Right boundary (less common but possible)
        if (transform.x >= SCREEN_RIGHT_BOUNDARY && velocity.dx > 0.0F) {
          velocity.dx = -velocity.dx; // Bounce left
        }
        // Left boundary (will be destroyed before hitting it usually)
        if (transform.x <= SCREEN_LEFT_BOUNDARY && velocity.dx < 0.0F) {
          velocity.dx = -velocity.dx; // Bounce right
        }
        
        // Update animation frames based on movement direction for robots
        if (world.hasComponent<ecs::Sprite>(entity)) {
          auto &sprite = world.getComponent<ecs::Sprite>(entity);
          if (sprite.spriteId == ecs::SpriteId::ENEMY_ROBOT) {
            // Frames 0-2: moving left (dx < 0)
            // Frames 3-5: moving right (dx > 0)
            if (velocity.dx < 0.0F) {
              // Moving left
              sprite.startFrame = 0;
              sprite.endFrame = 2;
            } else {
              // Moving right
              sprite.startFrame = 3;
              sprite.endFrame = 5;
            }
          }
        }
        
        // Shooting behavior: robots shoot periodically toward the player
        // Use pattern.amplitude to track shooting timer (repurposed since not used for bounce)
        pattern.amplitude += deltaTime;
        constexpr float ROBOT_SHOOT_INTERVAL = 2.5F; // Shoot every 2.5 seconds
        
        if (pattern.amplitude >= ROBOT_SHOOT_INTERVAL) {
          pattern.amplitude = 0.0F; // Reset timer
          
          // Find player to shoot at
          std::vector<ecs::Entity> players;
          ecs::ComponentSignature playerSig;
          playerSig.set(ecs::getComponentId<ecs::PlayerId>());
          world.getEntitiesWithSignature(playerSig, players);
          
          if (!players.empty()) {
            auto &robotPos = transform;
            auto &playerPos = world.getComponent<ecs::Transform>(players[0]);
            
            // Calculate direction to player
            float dx = playerPos.x - robotPos.x;
            float dy = playerPos.y - robotPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance > 0.0F) {
              // Create projectile
              constexpr float ROBOT_PROJECTILE_SPEED = 350.0F;
              float dirX = (dx / distance) * ROBOT_PROJECTILE_SPEED;
              float dirY = (dy / distance) * ROBOT_PROJECTILE_SPEED;
              
              ecs::Entity projectile = world.createEntity();
              
              // Position projectile at robot location
              ecs::Transform projTransform;
              projTransform.x = robotPos.x;
              projTransform.y = robotPos.y;
              projTransform.rotation = 0.0F;
              projTransform.scale = 0.4F; // Smaller projectiles
              world.addComponent(projectile, projTransform);
              
              // Set projectile velocity towards player
              ecs::Velocity projVelocity;
              projVelocity.dx = dirX;
              projVelocity.dy = dirY;
              world.addComponent(projectile, projVelocity);
              
              // Add sprite component (using robot projectile, single frame)
              ecs::Sprite projSprite;
              projSprite.spriteId = ecs::SpriteId::ROBOT_PROJECTILE;
              projSprite.width = 101;
              projSprite.height = 114;
              projSprite.animated = false; // No animation
              projSprite.frameCount = 1;
              projSprite.currentFrame = 0;
              projSprite.startFrame = 0;
              projSprite.endFrame = 0;
              projSprite.frameTime = 0.0F;
              projSprite.animationTimer = 0.0F;
              projSprite.reverseAnimation = false;
              projSprite.loop = false;
              world.addComponent(projectile, projSprite);
              
              // Add collider
              ecs::Collider projCollider;
              projCollider.width = 101.0F * 0.4F; // width * scale
              projCollider.height = 114.0F * 0.4F; // height * scale
              projCollider.shape = ecs::Collider::Shape::BOX;
              world.addComponent(projectile, projCollider);
              
              // Add owner component to mark this as enemy projectile
              ecs::Owner projOwner;
              projOwner.ownerId = entity; // The robot is the owner
              world.addComponent(projectile, projOwner);
              
              // Add networked component
              ecs::Networked net;
              net.networkId = projectile;
              world.addComponent(projectile, net);
            }
          }
        }
        
      } else if (pattern.patternType == "ground_walk") {
        // Walker enemy: stays on ground and shoots at player from horizontal range

        // Define shooting range and screen boundaries
        constexpr float SHOOTING_RANGE_MIN = 200.0F; // Minimum horizontal distance to maintain
        constexpr float SHOOTING_RANGE_MAX = 800.0F; // Maximum horizontal distance before approaching
        constexpr float SCREEN_LEFT_BOUNDARY = 50.0F;
        constexpr float SCREEN_RIGHT_BOUNDARY = 1820.0F;
        constexpr float GROUND_Y_POSITION = 950.0F; // Fixed Y position (ground level)
        constexpr float WALKER_SPEED = 150.0F;

        // Force walker to stay on ground
        transform.y = GROUND_Y_POSITION;
        velocity.dy = 0.0F;

        // Find player to track
        std::vector<ecs::Entity> players;
        ecs::ComponentSignature playerSig;
        playerSig.set(ecs::getComponentId<ecs::PlayerId>());
        world.getEntitiesWithSignature(playerSig, players);

        if (!players.empty()) {
          // Get walker's position
          auto &walkerPos = transform;

          // Get first player's position
          auto &playerPos = world.getComponent<ecs::Transform>(players[0]);

          // Calculate horizontal distance to player (ignore Y)
          float dx = playerPos.x - walkerPos.x;
          float horizontalDistance = std::abs(dx);

          // Movement logic: maintain shooting range horizontally only
          if (horizontalDistance > SHOOTING_RANGE_MAX) {
            // Too far: move towards player horizontally
            velocity.dx = (dx > 0.0F ? WALKER_SPEED : -WALKER_SPEED);
          } else if (horizontalDistance < SHOOTING_RANGE_MIN) {
            // Too close: move away from player horizontally
            velocity.dx = (dx > 0.0F ? -WALKER_SPEED : WALKER_SPEED);
          } else {
            // In optimal range: move slowly to track player
            velocity.dx = (dx / horizontalDistance) * (WALKER_SPEED * 0.3F);
          }

          // Keep walker within screen boundaries horizontally
          if (walkerPos.x < SCREEN_LEFT_BOUNDARY) {
            walkerPos.x = SCREEN_LEFT_BOUNDARY;
            velocity.dx = std::max(0.0F, velocity.dx); // Only allow moving right
          } else if (walkerPos.x > SCREEN_RIGHT_BOUNDARY) {
            walkerPos.x = SCREEN_RIGHT_BOUNDARY;
            velocity.dx = std::min(0.0F, velocity.dx); // Only allow moving left
          }

          // Update animation frames based on movement direction
          if (world.hasComponent<ecs::Sprite>(entity)) {
            auto &sprite = world.getComponent<ecs::Sprite>(entity);
            if (sprite.spriteId == ecs::SpriteId::ENEMY_WALKER) {
              if (velocity.dx > -0.1F) {
                // Moving left: use frames 3, 4, 5
                sprite.startFrame = 3;
                sprite.endFrame = 5;
              } else if (velocity.dx < 0.1F) {
                // Moving right: use frames 0, 1, 2
                sprite.startFrame = 0;
                sprite.endFrame = 2;
              } else {
                sprite.startFrame = 2;
                sprite.endFrame = 2;
              }
            }
          }

          // Shooting behavior when in horizontal range (can shoot at any Y)
          pattern.phase += deltaTime;
          constexpr float SHOOT_INTERVAL = 2.0F; // Shoot every 2 seconds

          // Calculate full distance including Y for shooting accuracy
          float dy = playerPos.y - walkerPos.y;
          float fullDistance = std::sqrt(dx * dx + dy * dy);

          if (pattern.phase >= SHOOT_INTERVAL && horizontalDistance <= SHOOTING_RANGE_MAX &&
              horizontalDistance >= SHOOTING_RANGE_MIN) {
            pattern.phase = 0.0F;

            if (fullDistance > 0.0F) {
              // Normalize direction and set projectile velocity (aim at player's Y position)
              constexpr float PROJECTILE_SPEED = 400.0F;
              float dirX = (dx / fullDistance) * PROJECTILE_SPEED;
              float dirY = (dy / fullDistance) * PROJECTILE_SPEED;

              // Create projectile entity
              ecs::Entity projectile = world.createEntity();

              // Position projectile at walker location
              ecs::Transform projTransform;
              projTransform.x = walkerPos.x;
              projTransform.y = walkerPos.y;
              projTransform.rotation = 0.0F;
              projTransform.scale = 0.5F; // Reduced size for projectile
              world.addComponent(projectile, projTransform);

              // Set projectile velocity towards player
              ecs::Velocity projVelocity;
              projVelocity.dx = dirX;
              projVelocity.dy = dirY;
              world.addComponent(projectile, projVelocity);

              // Add sprite component
              ecs::Sprite projSprite;
              projSprite.spriteId = ecs::SpriteId::WALKER_PROJECTILE;
              projSprite.width = 78; // 549 / 7 frames
              projSprite.height = 72;
              projSprite.animated = true;
              projSprite.frameCount = 4; // Use frames 0-3
              projSprite.currentFrame = 0;
              projSprite.startFrame = 0;
              projSprite.endFrame = 3;
              projSprite.frameTime = 0.08F; // Fast animation
              projSprite.animationTimer = 0.0F;
              projSprite.reverseAnimation = false;
              projSprite.loop = false; // Stay on frame 3
              world.addComponent(projectile, projSprite);

              // Add collider (proportional to sprite size with scale)
              ecs::Collider projCollider;
              projCollider.width = 78.0F * 0.5F; // width * scale
              projCollider.height = 72.0F * 0.5F; // height * scale
              projCollider.shape = ecs::Collider::Shape::BOX;
              world.addComponent(projectile, projCollider);

              // Add owner component to mark this as enemy projectile
              ecs::Owner projOwner;
              projOwner.ownerId = entity; // The walker is the owner
              world.addComponent(projectile, projOwner);

              // Add networked component
              ecs::Networked net;
              net.networkId = projectile;
              world.addComponent(projectile, net);
            }
          }
        } else {
          // No player found: stay still on ground
          velocity.dx = 0.0F;
          velocity.dy = 0.0F;
        }
      } else {
        // Default or "none" pattern: keep velocity as configured
        // velocity.dx and velocity.dy are already set from config
      }

      // Update rotation for yellow bee based on velocity direction
      if (world.hasComponent<ecs::Sprite>(entity)) {
        auto &sprite = world.getComponent<ecs::Sprite>(entity);
        if (sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW) {
          // Calculate rotation angle from velocity (in degrees)
          // Base sprite faces left, so add 180° to make it face right
          float targetAngle = std::atan2(velocity.dy, velocity.dx) * (180.0F / 3.14159265F) + 180.0F;

          // Smooth rotation: interpolate towards target angle
          float currentAngle = transform.rotation;
          float angleDiff = targetAngle - currentAngle;

          // Normalize angle difference to [-180, 180]
          while (angleDiff > 180.0F)
            angleDiff -= 360.0F;
          while (angleDiff < -180.0F)
            angleDiff += 360.0F;

          // Smooth rotation speed (degrees per second)
          constexpr float ROTATION_SPEED = 180.0F;
          float maxRotation = ROTATION_SPEED * deltaTime;

          if (std::abs(angleDiff) < maxRotation) {
            transform.rotation = targetAngle;
          } else {
            transform.rotation += (angleDiff > 0 ? maxRotation : -maxRotation);
          }

          // Normalize final angle to [0, 360)
          while (transform.rotation >= 360.0F)
            transform.rotation -= 360.0F;
          while (transform.rotation < 0.0F)
            transform.rotation += 360.0F;
        }
      }

      // Destroy if off-screen (left side), but not for walkers who stay on screen
      if (pattern.patternType != "ground_walk" && transform.x < OFFSCREEN_DESTROY_X) {
        world.destroyEntity(entity);
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Pattern>());
    sig.set(ecs::getComponentId<ecs::Velocity>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }

private:
  // AI behavior constants
  static constexpr float ENEMY_MOVE_SPEED = -384.0F;
  static constexpr float OFFSCREEN_DESTROY_X = -100.0F;
};

} // namespace server

#endif // SERVER_ENEMY_AI_SYSTEM_HPP_
