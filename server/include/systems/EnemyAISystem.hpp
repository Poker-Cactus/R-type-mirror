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
#include "../../../engineCore/include/ecs/components/Attraction.hpp"
#include "../../../engineCore/include/ecs/components/Collider.hpp"
#include "../../../engineCore/include/ecs/components/Lifetime.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Pattern.hpp"
#include "../../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_map>
#include <vector>

namespace server
{

struct SpriteFrame {
  int x, y, width, height;
};

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

    static const std::vector<SpriteFrame> babyGoblinFrames = [] {
      std::vector<SpriteFrame> f;
      f.push_back({100, 1, 26, 29}); // Sprite 0
      f.push_back({136, 1, 26, 29}); // Sprite 1
      f.push_back({65, 34, 25, 28}); // Sprite 2
      f.push_back({100, 34, 25, 28}); // Sprite 3
      f.push_back({137, 34, 25, 28}); // Sprite 4
      f.push_back({172, 34, 25, 28}); // Sprite 5
      f.push_back({32, 70, 31, 23}); // Sprite 6
      f.push_back({65, 70, 31, 22}); // Sprite 7
      f.push_back({166, 70, 31, 22}); // Sprite 8
      f.push_back({199, 70, 31, 23}); // Sprite 9
      f.push_back({1, 100, 23, 27}); // Sprite 10
      f.push_back({34, 100, 26, 27}); // Sprite 11
      f.push_back({202, 100, 26, 27}); // Sprite 12
      f.push_back({238, 100, 23, 27}); // Sprite 13
      f.push_back({1, 134, 23, 27}); // Sprite 14
      f.push_back({34, 134, 26, 27}); // Sprite 15
      f.push_back({202, 134, 26, 27}); // Sprite 16
      f.push_back({238, 134, 23, 27}); // Sprite 17
      f.push_back({32, 168, 31, 23}); // Sprite 18
      f.push_back({65, 169, 31, 22}); // Sprite 19
      f.push_back({166, 169, 31, 22}); // Sprite 20
      f.push_back({199, 168, 31, 23}); // Sprite 21
      f.push_back({65, 199, 25, 28}); // Sprite 22
      f.push_back({100, 199, 25, 28}); // Sprite 23
      f.push_back({137, 199, 25, 28}); // Sprite 24
      f.push_back({172, 199, 25, 28}); // Sprite 25
      f.push_back({100, 231, 26, 29}); // Sprite 26
      f.push_back({136, 231, 26, 29}); // Sprite 27
      return f;
    }();

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
          std::mt19937 rng(static_cast<unsigned int>(entity) + static_cast<unsigned int>(transform.x * 100.0F) +
                           static_cast<unsigned int>(transform.y * 100.0F));
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
      } else if (pattern.patternType == "boss_pattern") {
        // Boss behavior:
        // 1) Enter from the right with horizontal velocity.
        // 2) Once fully on-screen -> stop horizontal movement and switch to vertical-only
        //    allers-retours with variable speed (randomized interval + smooth lerp).
        // 3) Keep existing shooting behavior intact.

        constexpr float SCREEN_TOP_BOUNDARY = 0.0F;
        constexpr float SCREEN_BOTTOM_BOUNDARY = 1080.0F;
        constexpr float SCREEN_LEFT_BOUNDARY = 0.0F;
        constexpr float SCREEN_RIGHT_BOUNDARY = 1920.0F;
        constexpr float DEFAULT_ENTRY_MARGIN = 400.0F; // fallback if sprite info missing

        // per-boss runtime state (created lazily)
        struct BossState {
          bool verticalMode = false; // true once boss finished entering
          float speedChangeTimer = 0.0F; // accumulates time until next speed change
          float nextChangeInterval = 1.0F; // randomized interval between speed changes
          float targetSpeed = 150.0F; // current target vertical speed magnitude
        };

        static std::unordered_map<ecs::Entity, BossState> s_bossStates;
        auto &state = s_bossStates[entity];

        // Initialization (only used to pick an initial vertical direction if needed)
        if (pattern.phase == 0.0F) {
          pattern.phase = 1.0F;

          std::mt19937 rng(static_cast<unsigned int>(entity) + static_cast<unsigned int>(transform.x * 100.0F) +
                           static_cast<unsigned int>(transform.y * 100.0F));
          std::uniform_int_distribution<int> dist(0, 1);
          velocity.dy = (dist(rng) == 0 ? std::abs(velocity.dy) : -std::abs(velocity.dy));

          // seed initial randomized interval and speed
          std::uniform_real_distribution<float> ivar(0.8F, 2.0F);
          std::uniform_real_distribution<float> speedVar(100.0F, 280.0F);
          state.nextChangeInterval = ivar(rng);
          state.targetSpeed = speedVar(rng);
          state.speedChangeTimer = 0.0F;
        }

        // Determine X position at which the boss is "fully on-screen".
        float entryX = SCREEN_RIGHT_BOUNDARY - DEFAULT_ENTRY_MARGIN;
        if (world.hasComponent<ecs::Sprite>(entity)) {
          auto &sprite = world.getComponent<ecs::Sprite>(entity);
          float halfWidth = (sprite.width * (world.getComponent<ecs::Transform>(entity).scale));
          // assume transform.x is centered; subtract halfWidth to ensure fully visible
          entryX = SCREEN_RIGHT_BOUNDARY - halfWidth;
        }

        // If not yet in vertical mode, check for transition condition
        if (!state.verticalMode) {
          // Still entering: keep horizontal movement
          // If the boss has reached the entry X, switch to vertical movement
          if (transform.x <= entryX) {
            state.verticalMode = true;
            // lock X so boss doesn't drift
            transform.x = entryX;
            velocity.dx = 0.0F;

            // ensure we have a sensible starting vertical speed (preserve sign)
            float sign = (velocity.dy < 0.0F ? -1.0F : 1.0F);
            velocity.dy = sign * state.targetSpeed;

            // small safety clamp to keep boss away from exact borders
            transform.y = std::clamp(transform.y, SCREEN_TOP_BOUNDARY + 20.0F, SCREEN_BOTTOM_BOUNDARY - 20.0F);
          } else {
            // keep moving left while entering
            // (leave dx as configured in spawn/velocity)
          }
        } else {
          // VERTICAL MODE: boss performs allers-retours (up/down) with variable speed

          // Lock horizontal position
          transform.x = entryX;
          velocity.dx = 0.0F;

          // Update speed-change timer and pick a new target speed when interval elapsed
          state.speedChangeTimer += deltaTime;
          if (state.speedChangeTimer >= state.nextChangeInterval) {
            // randomize next interval and target speed
            std::mt19937 rng(static_cast<unsigned int>(entity) + static_cast<unsigned int>(transform.y * 100.0F) +
                             static_cast<unsigned int>(state.speedChangeTimer * 1000.0F));
            std::uniform_real_distribution<float> ivar(0.6F, 2.2F);
            std::uniform_real_distribution<float> speedVar(90.0F, 340.0F);
            state.nextChangeInterval = ivar(rng);
            state.targetSpeed = speedVar(rng);
            state.speedChangeTimer = 0.0F;
          }

          // Smoothly approach the target speed (lerp) to avoid instant jumps
          constexpr float SPEED_LERP = 4.0F; // higher = faster interpolation
          float curSpeed = std::abs(velocity.dy);
          float newSpeed = curSpeed + (state.targetSpeed - curSpeed) * std::min(1.0F, SPEED_LERP * deltaTime);
          velocity.dy = (velocity.dy < 0.0F ? -newSpeed : newSpeed);

          // Reverse direction on top/bottom hit (classic allers-retours)
          if (transform.y <= SCREEN_TOP_BOUNDARY + 10.0F && velocity.dy < 0.0F) {
            velocity.dy = -velocity.dy;
          }
          if (transform.y >= SCREEN_BOTTOM_BOUNDARY - 10.0F && velocity.dy > 0.0F) {
            velocity.dy = -velocity.dy;
          }

          // Optional: small positional clamp to avoid sticking out
          transform.y = std::clamp(transform.y, SCREEN_TOP_BOUNDARY + 1.0F, SCREEN_BOTTOM_BOUNDARY - 1.0F);
        }

        // --- Keep existing shooting behavior intact (unchanged) ---
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
              projTransform.rotation = 1.0F;
              projTransform.scale = 3.0F; // Smaller projectiles
              world.addComponent(projectile, projTransform);

              // Set projectile velocity towards player
              ecs::Velocity projVelocity;
              projVelocity.dx = dirX;
              projVelocity.dy = dirY;
              world.addComponent(projectile, projVelocity);

              // Add sprite component (using robot projectile, single frame)
              ecs::Sprite projSprite;
              projSprite.spriteId = ecs::SpriteId::BOSS_DOBKERATOP_SHOOT;
              projSprite.width = 34;
              projSprite.height = 34;
              projSprite.animated = true;
              projSprite.frameCount = 3;
              projSprite.currentFrame = 0;
              projSprite.startFrame = 0;
              projSprite.endFrame = 2;
              projSprite.frameTime = 0.08F;
              projSprite.reverseAnimation = false;
              projSprite.loop = true;
              world.addComponent(projectile, projSprite);

              // Add collider
              ecs::Collider projCollider;
              projCollider.width = 34.0F;
              projCollider.height = 34.0F;
              projCollider.shape = ecs::Collider::Shape::CIRCLE;
              world.addComponent(projectile, projCollider);

              // Add owner component to mark this as enemy projectile
              ecs::Owner projOwner;
              projOwner.ownerId = entity; // The robot is the owner
              world.addComponent(projectile, projOwner);

              // Add attraction component
              ecs::Attraction projAttraction;
              projAttraction.force = 500.0F;
              projAttraction.radius = 300.0F;
              world.addComponent(projectile, projAttraction);

              // Add networked component
              ecs::Networked net;
              net.networkId = projectile;
              world.addComponent(projectile, net);
            }
          }
        }

      } else if (pattern.patternType == "boss_brocolis_pattern") {
        struct BrocolisState {
          bool hasEntered = false; // Pour le Boss : est-il arrivé au centre ?
          bool isHatching = false; // Pour l'Œuf/Projectile : est-il en train d'éclore ?
          float hatchingTimer = 0.0F; // Timer pour l'animation d'éclosion
        };
        static std::unordered_map<ecs::Entity, BrocolisState> s_brocolisStates;
        auto &state = s_brocolisStates[entity];

        // Identifier le type d'entité selon son Sprite actuel
        bool isProjectile = false;
        bool isHatchingEgg = false;

        if (world.hasComponent<ecs::Sprite>(entity)) {
          auto &sprite = world.getComponent<ecs::Sprite>(entity);
          if (sprite.spriteId == ecs::SpriteId::BOSS_BROCOLIS_SHOOT) {
            isProjectile = true;
          } else if (sprite.spriteId == ecs::SpriteId::BOSS_BROCOLIS_ECLOSION) {
            isHatchingEgg = true;
          }
        }

        if (isProjectile || isHatchingEgg) {
          // =========================================================
          // LOGIQUE DU PROJECTILE / OEUF
          // 1. Vole vers le joueur (SHOOT)
          // 2. Si touché -> Devient OEUF (ECLOSION) et s'arrête
          // 3. Animation ECLOSION -> Spawn Mini Boss
          // =========================================================

          auto &sprite = world.getComponent<ecs::Sprite>(entity);

          // Cas 1 : C'est un projectile en vol (BOSS_BROCOLIS_SHOOT)
          if (isProjectile && !state.isHatching) {
            // Rotation du sprite selon la vélocité (Optionnel, pour le réalisme)
            if (velocity.dx != 0 || velocity.dy != 0) {
              // Ajustement simple ou rotation complète via Transform
            }

            // Vérifier si le projectile a été touché par le joueur (Dégâts reçus)
            if (world.hasComponent<ecs::Health>(entity)) {
              const auto &hp = world.getComponent<ecs::Health>(entity);
              if (hp.hp < hp.maxHp) {
                // Seuls les projectiles tirés par le BOSS "parent" (grande échelle) peuvent éclore.
                bool ownerIsParentBoss = false;
                if (world.hasComponent<ecs::Owner>(entity)) {
                  const auto &ownerComp = world.getComponent<ecs::Owner>(entity);
                  if (world.isAlive(ownerComp.ownerId) && world.hasComponent<ecs::Transform>(ownerComp.ownerId) &&
                      world.hasComponent<ecs::Sprite>(ownerComp.ownerId)) {
                    const auto &ownerTrans = world.getComponent<ecs::Transform>(ownerComp.ownerId);
                    const auto &ownerSpr = world.getComponent<ecs::Sprite>(ownerComp.ownerId);
                    if (ownerSpr.spriteId == ecs::SpriteId::BOSS_BROCOLIS && ownerTrans.scale > 2.0F) {
                      ownerIsParentBoss = true;
                    }
                  }
                }

                if (!ownerIsParentBoss) {
                  // Petit boss : ne PAS démarrer l'animation d'éclosion — laisser la destruction normale agir
                } else {
                  // TRANSITION : Le projectile est touché, il devient un œuf qui éclot
                  state.isHatching = true;

                  // 1. Changer le sprite visuel
                  sprite.spriteId = ecs::SpriteId::BOSS_BROCOLIS_ECLOSION;

                  // 2. Arrêter le mouvement immédiatement
                  velocity.dx = 0.0F;
                  velocity.dy = 0.0F;

                  // 3. Configurer l'animation d'éclosion (Ouverture 3 -> 0)
                  sprite.animated = true;
                  sprite.reverseAnimation = true;
                  sprite.startFrame = 0;
                  sprite.endFrame = 3;
                  sprite.currentFrame = 3; // Commence fermé
                  sprite.loop = false;
                  sprite.frameTime = 0.15F;

                  // 4. Reset Timer
                  state.hatchingTimer = 0.0F;

                  // 5. Désactiver le Collider ou le mettre en Trigger pour éviter qu'il ne re-prenne des dégâts
                  // (Optionnel selon votre moteur, ici on laisse mais on ignore les dégâts suivants)
                }
              }
            }
          }
          // Cas 2 : C'est un œuf en cours d'éclosion (BOSS_BROCOLIS_ECLOSION)
          else if (state.isHatching || isHatchingEgg) {
            // S'assurer que la vélocité reste à 0
            velocity.dx = 0.0F;
            velocity.dy = 0.0F;

            // Avancer le timer
            state.hatchingTimer += deltaTime;
            constexpr float HATCH_DURATION = 0.6F; // Durée de l'anim

            if (state.hatchingTimer >= HATCH_DURATION) {
              // FIN DE L'ÉCLOSION -> SPAWN DU MINI BOSS

              ecs::Entity newBoss = world.createEntity();

              // Position
              ecs::Transform bossTrans;
              bossTrans.x = transform.x;
              bossTrans.y = transform.y;
              bossTrans.scale = 1.5F; // Mini Boss
              world.addComponent(newBoss, bossTrans);

              // Sprite
              ecs::Sprite bossSprite;
              bossSprite.spriteId = ecs::SpriteId::BOSS_BROCOLIS;
              bossSprite.width = 33;
              bossSprite.height = 34;
              bossSprite.animated = true;
              bossSprite.frameCount = 4;
              bossSprite.startFrame = 0;
              bossSprite.endFrame = 3;
              bossSprite.currentFrame = 0;
              bossSprite.frameTime = 0.15F;
              bossSprite.loop = true;
              world.addComponent(newBoss, bossSprite);

              // Physique
              ecs::Velocity bossVel;
              bossVel.dx = 0.0F;
              bossVel.dy = 0.0F;
              world.addComponent(newBoss, bossVel);

              ecs::Collider bossCol;
              bossCol.width = 33.0F * 1.5F;
              bossCol.height = 34.0F * 1.5F;
              world.addComponent(newBoss, bossCol);

              // Santé
              ecs::Health bossHp;
              bossHp.maxHp = 200;
              bossHp.hp = 200;
              world.addComponent(newBoss, bossHp);

              // IA (Récursive)
              ecs::Pattern bossPat;
              bossPat.patternType = "boss_brocolis_pattern";
              bossPat.phase = 0.0F;
              world.addComponent(newBoss, bossPat);

              ecs::Networked net;
              net.networkId = newBoss;
              world.addComponent(newBoss, net);

              // Détruire l'œuf/projectile transformé
              world.destroyEntity(entity);
              s_brocolisStates.erase(entity);
            }
          }

        } else {
          // =========================================================
          // LOGIQUE DU BOSS (Brocolis Mère/Père)
          // =========================================================

          constexpr float TARGET_ENTER_X = 960.0F;
          constexpr float TARGET_ENTER_Y = 200.0F;
          constexpr float ENTER_SPEED = 300.0F;
          constexpr float ENTER_THRESHOLD = 10.0F;

          if (!state.hasEntered) {
            // PHASE 1 : ENTRÉE
            float dx = TARGET_ENTER_X - transform.x;
            float dy = TARGET_ENTER_Y - transform.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < ENTER_THRESHOLD) {
              state.hasEntered = true;
              velocity.dx = 0.0F;
              velocity.dy = 0.0F;
              transform.x = TARGET_ENTER_X;
              transform.y = TARGET_ENTER_Y;
            } else {
              velocity.dx = (dx / dist) * ENTER_SPEED;
              velocity.dy = (dy / dist) * ENTER_SPEED;
            }

          } else {
            // PHASE 2 : COMBAT
            constexpr float PREFERRED_DISTANCE = 600.0F;
            constexpr float MOVE_SPEED = 200.0F;
            constexpr float SHOOT_INTERVAL = 5.0F; // Fréquence de tir
            constexpr float SCREEN_MARGIN = 50.0F;

            // 1. Mouvement (Reste identique : flottement/évitement)
            std::vector<ecs::Entity> players;
            ecs::ComponentSignature playerSig;
            playerSig.set(ecs::getComponentId<ecs::PlayerId>());
            world.getEntitiesWithSignature(playerSig, players);

            float targetDx = 0.0F;
            float targetDy = 0.0F;

            if (!players.empty()) {
              auto &playerPos = world.getComponent<ecs::Transform>(players[0]);
              float dx = transform.x - playerPos.x;
              float dy = transform.y - playerPos.y;
              float dist = std::sqrt(dx * dx + dy * dy);

              if (dist > 0.0F) {
                dx /= dist;
                dy /= dist;
              }

              if (dist < PREFERRED_DISTANCE) {
                targetDx = dx * MOVE_SPEED;
                targetDy = dy * MOVE_SPEED;
              } else {
                float driftX = std::cos(pattern.phase * 0.5F);
                float driftY = std::sin(pattern.phase * 0.8F);
                targetDx = driftX * (MOVE_SPEED * 0.5F);
                targetDy = driftY * (MOVE_SPEED * 0.5F);
              }
            }

            velocity.dx += (targetDx - velocity.dx) * 2.0F * deltaTime;
            velocity.dy += (targetDy - velocity.dy) * 2.0F * deltaTime;

            if (transform.x < SCREEN_MARGIN && velocity.dx < 0)
              velocity.dx = -velocity.dx;
            if (transform.x > 1920.0F - SCREEN_MARGIN && velocity.dx > 0)
              velocity.dx = -velocity.dx;
            if (transform.y < SCREEN_MARGIN && velocity.dy < 0)
              velocity.dy = -velocity.dy;
            if (transform.y > 1080.0F - SCREEN_MARGIN && velocity.dy > 0)
              velocity.dy = -velocity.dy;

            // 2. TIR : comportement différent selon taille (parent boss vs mini‑boss)
            if (transform.scale > 2.0F) {
              // Parent boss (inchangé)
              pattern.phase += deltaTime;

              if (pattern.phase >= SHOOT_INTERVAL) {
                pattern.phase = 0.0F;

                // Trouver le joueur pour viser
                float shootDirX = 0.0F;
                float shootDirY = 1.0F; // Par défaut vers le bas

                if (!players.empty()) {
                  auto &pPos = world.getComponent<ecs::Transform>(players[0]);
                  float pdx = pPos.x - transform.x;
                  float pdy = pPos.y - transform.y;
                  float pdist = std::sqrt(pdx * pdx + pdy * pdy);
                  if (pdist > 0) {
                    shootDirX = pdx / pdist;
                    shootDirY = pdy / pdist;
                  }
                }

                ecs::Entity proj = world.createEntity();
                ecs::Transform projTrans;
                projTrans.x = transform.x;
                projTrans.y = transform.y + 40.0F;
                projTrans.scale = 0.75F; // reduced for smaller visual projectile
                world.addComponent(proj, projTrans);

                // Utilisation du sprite PROJECTILE (SHOOT) au départ
                ecs::Sprite projSprite;
                projSprite.spriteId = ecs::SpriteId::BOSS_BROCOLIS_SHOOT;
                projSprite.width = 33;
                projSprite.height = 31;
                projSprite.animated = true;
                projSprite.frameCount = 4;
                projSprite.currentFrame = 0;
                projSprite.startFrame = 0;
                projSprite.endFrame = 3;
                projSprite.frameTime = 0.08F;
                projSprite.animationTimer = 0.0F;
                projSprite.reverseAnimation = false;
                projSprite.loop = true;
                world.addComponent(proj, projSprite);

                // Vélocité vers le joueur (slightly faster)
                constexpr float PROJ_SPEED = 300.0F;
                ecs::Velocity projVel;
                projVel.dx = shootDirX * PROJ_SPEED;
                projVel.dy = shootDirY * PROJ_SPEED;
                world.addComponent(proj, projVel);

                // Le projectile utilise le MÊME pattern système pour gérer sa transformation
                ecs::Pattern projPat;
                projPat.patternType = "boss_brocolis_pattern"; // parent projectiles can hatch
                world.addComponent(proj, projPat);

                // Santé (Pour détecter si le joueur tire dessus)
                ecs::Health projHp;
                projHp.maxHp = 10; // Fragile
                projHp.hp = 10;
                world.addComponent(proj, projHp);

                // Collider (Pour faire des dégâts au joueur ET être touché)
                ecs::Collider projCol;
                projCol.width = 33.0F * projTrans.scale; // match visual scale
                projCol.height = 31.0F * projTrans.scale; // match visual scale
                projCol.shape = ecs::Collider::Shape::CIRCLE;
                world.addComponent(proj, projCol);

                // Owner : Défini pour éviter que le boss ne se blesse lui-même
                ecs::Owner owner;
                owner.ownerId = entity;
                world.addComponent(proj, owner);

                ecs::Networked net;
                net.networkId = proj;
                world.addComponent(proj, net);
              }

            } else if (transform.scale > 1.0F) {
              // Mini-boss : tire aussi, mais ses projectiles NE DOIVENT PAS éclore
              constexpr float MINI_SHOOT_INTERVAL = 3.0F; // plus fréquent
              pattern.phase += deltaTime;

              if (pattern.phase >= MINI_SHOOT_INTERVAL) {
                pattern.phase = 0.0F;

                // Vise le joueur (même logique)
                float shootDirX = 0.0F;
                float shootDirY = 1.0F;
                if (!players.empty()) {
                  auto &pPos = world.getComponent<ecs::Transform>(players[0]);
                  float pdx = pPos.x - transform.x;
                  float pdy = pPos.y - transform.y;
                  float pdist = std::sqrt(pdx * pdx + pdy * pdy);
                  if (pdist > 0) {
                    shootDirX = pdx / pdist;
                    shootDirY = pdy / pdist;
                  }
                }

                ecs::Entity proj = world.createEntity();
                ecs::Transform projTrans;
                projTrans.x = transform.x;
                projTrans.y = transform.y + 28.0F; // légèrement différent
                projTrans.scale = 0.65F; // smaller than parent projectiles
                world.addComponent(proj, projTrans);

                ecs::Sprite projSprite;
                projSprite.spriteId = ecs::SpriteId::BOSS_BROCOLIS_SHOOT;
                projSprite.width = 33;
                projSprite.height = 31;
                projSprite.animated = true;
                projSprite.frameCount = 4;
                projSprite.currentFrame = 0;
                projSprite.startFrame = 0;
                projSprite.endFrame = 3;
                projSprite.frameTime = 0.08F;
                projSprite.loop = true;
                world.addComponent(proj, projSprite);

                // Vitesse et résistance moindres (slightly faster than before)
                constexpr float PROJ_SPEED_CHILD = 240.0F;
                ecs::Velocity projVel;
                projVel.dx = shootDirX * PROJ_SPEED_CHILD;
                projVel.dy = shootDirY * PROJ_SPEED_CHILD;
                world.addComponent(proj, projVel);

                // IMPORTANT: utilise le MÊME sprite/pattern mais son owner est un mini-boss
                // La logique d'éclosion vérifie maintenant l'owner pour décider du comportement.
                ecs::Pattern projPat;
                projPat.patternType = "boss_brocolis_pattern";
                world.addComponent(proj, projPat);

                ecs::Health projHp;
                projHp.maxHp = 6;
                projHp.hp = 6;
                world.addComponent(proj, projHp);

                ecs::Collider projCol;
                projCol.width = 28.0F * projTrans.scale;
                projCol.height = 28.0F * projTrans.scale;
                projCol.shape = ecs::Collider::Shape::CIRCLE;
                world.addComponent(proj, projCol);

                ecs::Owner owner;
                owner.ownerId = entity;
                world.addComponent(proj, owner);

                ecs::Networked net;
                net.networkId = proj;
                world.addComponent(proj, net);
              }
            }
          }
        }
      } else if (pattern.patternType == "boss_evangelic_pattern") {
        bool isProjectile = world.hasComponent<ecs::Owner>(entity);
        std::vector<ecs::Entity> players;
        ecs::ComponentSignature playerSig;
        playerSig.set(ecs::getComponentId<ecs::PlayerId>());
        world.getEntitiesWithSignature(playerSig, players);

        if (isProjectile) {
          // Structure pour gérer l'état du projectile boomerang
          struct BoomerangState {
            float spawnX = 0.0F;
            float spawnY = 0.0F;
            float timer = 0.0F;
            bool returning = false;
            bool hasReachedSpawn = false;
          };
          static std::unordered_map<ecs::Entity, BoomerangState> s_boomerangStates;
          auto &bState = s_boomerangStates[entity];

          constexpr float PROJ_SPEED = 250.0F;
          constexpr float BOOMERANG_TIMER = 7.0F;

          // Initialiser la position de spawn au premier frame
          if (bState.timer == 0.0F && bState.spawnX == 0.0F && bState.spawnY == 0.0F) {
            bState.spawnX = transform.x;
            bState.spawnY = transform.y;
          }

          // Incrémenter le timer
          bState.timer += deltaTime;

          // Phase 1 : Aller vers le joueur (0-7 secondes)
          if (!bState.returning && bState.timer < BOOMERANG_TIMER) {
            if (!players.empty()) {
              auto &playerPos = world.getComponent<ecs::Transform>(players[0]);

              float dx = playerPos.x - transform.x;
              float dy = playerPos.y - transform.y;
              float dist = std::sqrt(dx * dx + dy * dy);

              if (dist > 0.0F) {
                velocity.dx = (dx / dist) * PROJ_SPEED;
                velocity.dy = (dy / dist) * PROJ_SPEED;
                float angleRad = std::atan2(velocity.dy, velocity.dx);
                transform.rotation = angleRad * (180.0F / 3.14159F);
              }
            } else {
              if (velocity.dx == 0 && velocity.dy == 0) {
                velocity.dx = -PROJ_SPEED;
              }
            }
          }
          // Phase 2 : Retourner au point de spawn (après 7 secondes)
          else if (bState.timer >= BOOMERANG_TIMER && !bState.hasReachedSpawn) {
            bState.returning = true;

            float dx = bState.spawnX - transform.x;
            float dy = bState.spawnY - transform.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            // Si on est arrivé au point de spawn (à 20 pixels près)
            if (dist < 20.0F) {
              bState.hasReachedSpawn = true;
              bState.timer = 0.0F; // Reset le timer pour repartir
            } else if (dist > 0.0F) {
              velocity.dx = (dx / dist) * PROJ_SPEED;
              velocity.dy = (dy / dist) * PROJ_SPEED;
              float angleRad = std::atan2(velocity.dy, velocity.dx);
              transform.rotation = angleRad * (180.0F / 3.14159F);
            }
          }
          // Phase 3 : Repartir vers le joueur après être revenu au spawn
          else if (bState.hasReachedSpawn) {
            if (!players.empty()) {
              auto &playerPos = world.getComponent<ecs::Transform>(players[0]);

              float dx = playerPos.x - transform.x;
              float dy = playerPos.y - transform.y;
              float dist = std::sqrt(dx * dx + dy * dy);

              if (dist > 0.0F) {
                velocity.dx = (dx / dist) * PROJ_SPEED;
                velocity.dy = (dy / dist) * PROJ_SPEED;
                float angleRad = std::atan2(velocity.dy, velocity.dx);
                transform.rotation = angleRad * (180.0F / 3.14159F);
              }
            }
          }

          // Détruire si vraiment trop loin (sécurité)
          if (transform.x < -400.0F || transform.x > 2320.0F || transform.y < -400.0F || transform.y > 1480.0F) {
            world.destroyEntity(entity);
            s_boomerangStates.erase(entity);
          }
        } else {
          // CODE DU BOSS
          constexpr float PREFERRED_X = 1400.0F;
          constexpr float PREFERRED_Y = 540.0F;
          constexpr float X_SMOOTH = 3.0F;
          constexpr float HOVER_AMPL = 120.0F;
          constexpr float HOVER_FREQ = 1.2F;
          constexpr float EDGE_SPAWN_INTERVAL = 2.0F;
          constexpr float EDGE_MARGIN = 24.0F;
          constexpr int MAX_PROJECTILES = 5;

          float targetDx = (PREFERRED_X - transform.x) * X_SMOOTH;
          velocity.dx += (targetDx - velocity.dx) * std::min(1.0F, deltaTime * 4.0F);

          pattern.phase += deltaTime * HOVER_FREQ;
          float hoverTargetY = PREFERRED_Y + std::sin(pattern.phase) * HOVER_AMPL;
          float desiredDy = (hoverTargetY - transform.y) * 2.0F;
          velocity.dy += (desiredDy - velocity.dy) * (0.5F * deltaTime);

          pattern.amplitude += deltaTime;

          if (pattern.amplitude >= EDGE_SPAWN_INTERVAL) {
            pattern.amplitude = 0.0F;

            // Compter les projectiles existants du boss
            std::vector<ecs::Entity> allEntities;
            ecs::ComponentSignature projSig;
            projSig.set(ecs::getComponentId<ecs::Owner>());
            projSig.set(ecs::getComponentId<ecs::Pattern>());
            world.getEntitiesWithSignature(projSig, allEntities);

            int currentProjectiles = 0;
            for (auto e : allEntities) {
              if (world.hasComponent<ecs::Owner>(e)) {
                auto &owner = world.getComponent<ecs::Owner>(e);
                if (owner.ownerId == entity) {
                  currentProjectiles++;
                }
              }
            }

            // Ne spawn que si on n'a pas atteint le maximum
            if (!players.empty() && currentProjectiles < MAX_PROJECTILES) {
              auto &targetPos = world.getComponent<ecs::Transform>(players[0]);

              // Spawn seulement 2 projectiles par cycle (un en haut, un en bas)
              int toSpawn = std::min(2, MAX_PROJECTILES - currentProjectiles);

              for (int side = 0; side < toSpawn; ++side) {
                ecs::Entity proj = world.createEntity();

                ecs::Transform projTrans;
                projTrans.x = transform.x;
                projTrans.y = (side == 0) ? EDGE_MARGIN : (1080.0F - EDGE_MARGIN);
                projTrans.rotation = 0.0F;
                projTrans.scale = 3.0F;
                world.addComponent(proj, projTrans);

                float dx = targetPos.x - projTrans.x;
                float dy = targetPos.y - projTrans.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                float dirX = -1.0F;
                float dirY = 0.0F;

                if (dist > 0.0F) {
                  dirX = dx / dist;
                  dirY = dy / dist;
                }

                constexpr float INITIAL_SPEED = 250.0F;
                ecs::Velocity projVel;
                projVel.dx = dirX * INITIAL_SPEED;
                projVel.dy = dirY * INITIAL_SPEED;
                world.addComponent(proj, projVel);

                ecs::Pattern projPattern;
                projPattern.patternType = "boss_evangelic_pattern";
                world.addComponent(proj, projPattern);

                ecs::Sprite projSprite;
                projSprite.spriteId = ecs::SpriteId::BOSS_EVANGELIC_SHOOT;
                projSprite.width = 32;
                projSprite.height = 30;
                projSprite.animated = true;
                projSprite.frameCount = 6;
                projSprite.currentFrame = 0;
                projSprite.startFrame = 0;
                projSprite.endFrame = 5;
                projSprite.frameTime = 0.08F;
                projSprite.loop = true;
                world.addComponent(proj, projSprite);

                ecs::Collider projCol;
                projCol.width = 32.0F * projTrans.scale;
                projCol.height = 30.0F * projTrans.scale;
                projCol.shape = ecs::Collider::Shape::CIRCLE;
                world.addComponent(proj, projCol);

                ecs::Health projHp;
                projHp.maxHp = 12;
                projHp.hp = 12;
                world.addComponent(proj, projHp);

                ecs::Owner owner;
                owner.ownerId = entity;
                world.addComponent(proj, owner);

                ecs::Networked net;
                net.networkId = proj;
                world.addComponent(proj, net);
              }
            }
          }
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
  // Subscription handle for DamageEvent
  ecs::EventListenerHandle m_damageHandle;

  // AI behavior constants
  static constexpr float ENEMY_MOVE_SPEED = -384.0F;
  static constexpr float OFFSCREEN_DESTROY_X = -100.0F;
};

} // namespace server

#endif // SERVER_ENEMY_AI_SYSTEM_HPP_
