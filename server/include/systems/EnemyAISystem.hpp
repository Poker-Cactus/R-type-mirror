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
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <cmath>
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
      } else {
        // Default or "none" pattern: keep velocity as configured
        // velocity.dx and velocity.dy are already set from config
      }
      
      // Update rotation for yellow bee based on velocity direction
      if (world.hasComponent<ecs::Sprite>(entity)) {
        auto& sprite = world.getComponent<ecs::Sprite>(entity);
        if (sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW) {
          // Calculate rotation angle from velocity (in degrees)
          // Base sprite faces left, so add 180° to make it face right
          float targetAngle = std::atan2(velocity.dy, velocity.dx) * (180.0F / 3.14159265F) + 180.0F;
          
          // Smooth rotation: interpolate towards target angle
          float currentAngle = transform.rotation;
          float angleDiff = targetAngle - currentAngle;
          
          // Normalize angle difference to [-180, 180]
          while (angleDiff > 180.0F) angleDiff -= 360.0F;
          while (angleDiff < -180.0F) angleDiff += 360.0F;
          
          // Smooth rotation speed (degrees per second)
          constexpr float ROTATION_SPEED = 180.0F;
          float maxRotation = ROTATION_SPEED * deltaTime;
          
          if (std::abs(angleDiff) < maxRotation) {
            transform.rotation = targetAngle;
          } else {
            transform.rotation += (angleDiff > 0 ? maxRotation : -maxRotation);
          }
          
          // Normalize final angle to [0, 360)
          while (transform.rotation >= 360.0F) transform.rotation -= 360.0F;
          while (transform.rotation < 0.0F) transform.rotation += 360.0F;
        }
      }

      // Destroy if off-screen (left side)
      if (transform.x < OFFSCREEN_DESTROY_X) {
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
