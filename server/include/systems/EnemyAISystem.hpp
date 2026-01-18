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

    for (auto entity : entities) {
      // Note: These references are valid at the start, but MAY become invalid
      // if world.addComponent is called inside the loop (vector reallocation).
      auto &transform = world.getComponent<ecs::Transform>(entity);
      auto &velocity = world.getComponent<ecs::Velocity>(entity);
      auto &pattern = world.getComponent<ecs::Pattern>(entity);

      // Apply pattern-based movement
      if (pattern.patternType == "sine_wave") {
        velocity.dx = ENEMY_MOVE_SPEED;
        pattern.phase += deltaTime * pattern.frequency;
        velocity.dy = pattern.amplitude * pattern.frequency * std::cos(pattern.phase);

      } else if (pattern.patternType == "zigzag") {
        float relativeY = transform.y - pattern.phase;
        if (pattern.phase == 0.0F) {
          pattern.phase = transform.y;
          relativeY = 0.0F;
        }
        if (relativeY > pattern.amplitude) {
          velocity.dy = -std::abs(velocity.dy);
        } else if (relativeY < -pattern.amplitude) {
          velocity.dy = std::abs(velocity.dy);
        }

      } else if (pattern.patternType == "straight") {
        velocity.dx = ENEMY_MOVE_SPEED;
        velocity.dy = 0.0F;

      } else if (pattern.patternType == "bounce") {
        constexpr float SCREEN_TOP_BOUNDARY = 0.0F;
        constexpr float SCREEN_BOTTOM_BOUNDARY = 1080.0F;
        constexpr float SCREEN_LEFT_BOUNDARY = 0.0F;
        constexpr float SCREEN_RIGHT_BOUNDARY = 1920.0F;

        if (pattern.phase == 0.0F) {
          pattern.phase = 1.0F;
          std::mt19937 rng(static_cast<unsigned int>(entity) + static_cast<unsigned int>(transform.x * 100.0F) +
                           static_cast<unsigned int>(transform.y * 100.0F));
          std::uniform_int_distribution<int> dist(0, 1);
          if (dist(rng) == 0)
            velocity.dy = std::abs(velocity.dy);
          else
            velocity.dy = -std::abs(velocity.dy);
        }
        if (transform.y <= SCREEN_TOP_BOUNDARY && velocity.dy < 0.0F)
          velocity.dy = -velocity.dy;
        if (transform.y >= SCREEN_BOTTOM_BOUNDARY && velocity.dy > 0.0F)
          velocity.dy = -velocity.dy;
        if (transform.x >= SCREEN_RIGHT_BOUNDARY && velocity.dx > 0.0F)
          velocity.dx = -velocity.dx;
        if (transform.x <= SCREEN_LEFT_BOUNDARY && velocity.dx < 0.0F)
          velocity.dx = -velocity.dx;

        if (world.hasComponent<ecs::Sprite>(entity)) {
          auto &sprite = world.getComponent<ecs::Sprite>(entity);
          if (sprite.spriteId == ecs::SpriteId::ENEMY_ROBOT) {
            if (velocity.dx < 0.0F) {
              sprite.startFrame = 0;
              sprite.endFrame = 2;
            } else {
              sprite.startFrame = 3;
              sprite.endFrame = 5;
            }
          }
        }

        pattern.amplitude += deltaTime;
        constexpr float ROBOT_SHOOT_INTERVAL = 2.5F;
        if (pattern.amplitude >= ROBOT_SHOOT_INTERVAL) {
          pattern.amplitude = 0.0F;
          std::vector<ecs::Entity> players;
          ecs::ComponentSignature playerSig;
          playerSig.set(ecs::getComponentId<ecs::PlayerId>());
          world.getEntitiesWithSignature(playerSig, players);

          if (!players.empty()) {
            // Copy values to avoid invalid references if reallocation occurs
            float robotX = transform.x;
            float robotY = transform.y;
            auto &playerPos = world.getComponent<ecs::Transform>(players[0]);
            float targetX = playerPos.x;
            float targetY = playerPos.y;

            float dx = targetX - robotX;
            float dy = targetY - robotY;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 0.0F) {
              constexpr float ROBOT_PROJECTILE_SPEED = 350.0F;
              float dirX = (dx / distance) * ROBOT_PROJECTILE_SPEED;
              float dirY = (dy / distance) * ROBOT_PROJECTILE_SPEED;

              ecs::Entity projectile = world.createEntity();
              ecs::Transform projTransform;
              projTransform.x = robotX;
              projTransform.y = robotY;
              projTransform.rotation = 0.0F;
              projTransform.scale = 0.4F;
              world.addComponent(projectile, projTransform);

              ecs::Velocity projVelocity;
              projVelocity.dx = dirX;
              projVelocity.dy = dirY;
              world.addComponent(projectile, projVelocity);

              ecs::Sprite projSprite;
              projSprite.spriteId = ecs::SpriteId::ROBOT_PROJECTILE;
              projSprite.width = 101;
              projSprite.height = 114;
              projSprite.animated = false;
              projSprite.frameCount = 1;
              projSprite.currentFrame = 0;
              projSprite.startFrame = 0;
              projSprite.endFrame = 0;
              projSprite.frameTime = 0.0F;
              projSprite.animationTimer = 0.0F;
              projSprite.reverseAnimation = false;
              projSprite.loop = false;
              world.addComponent(projectile, projSprite);

              ecs::Collider projCollider;
              projCollider.width = 101.0F * 0.4F;
              projCollider.height = 114.0F * 0.4F;
              projCollider.shape = ecs::Collider::Shape::BOX;
              world.addComponent(projectile, projCollider);

              ecs::Owner projOwner;
              projOwner.ownerId = entity;
              world.addComponent(projectile, projOwner);

              ecs::Networked net;
              net.networkId = projectile;
              world.addComponent(projectile, net);
            }
          }
        }

      } else if (pattern.patternType == "ground_walk") {
        constexpr float SHOOTING_RANGE_MIN = 200.0F;
        constexpr float SHOOTING_RANGE_MAX = 800.0F;
        constexpr float SCREEN_LEFT_BOUNDARY = 50.0F;
        constexpr float SCREEN_RIGHT_BOUNDARY = 1820.0F;
        constexpr float GROUND_Y_POSITION = 950.0F;
        constexpr float WALKER_SPEED = 150.0F;

        transform.y = GROUND_Y_POSITION;
        velocity.dy = 0.0F;

        std::vector<ecs::Entity> players;
        ecs::ComponentSignature playerSig;
        playerSig.set(ecs::getComponentId<ecs::PlayerId>());
        world.getEntitiesWithSignature(playerSig, players);

        if (!players.empty()) {
          auto &playerPos = world.getComponent<ecs::Transform>(players[0]);
          float dx = playerPos.x - transform.x;
          float horizontalDistance = std::abs(dx);

          if (horizontalDistance > SHOOTING_RANGE_MAX) {
            velocity.dx = (dx > 0.0F ? WALKER_SPEED : -WALKER_SPEED);
          } else if (horizontalDistance < SHOOTING_RANGE_MIN) {
            velocity.dx = (dx > 0.0F ? -WALKER_SPEED : WALKER_SPEED);
          } else {
            velocity.dx = (dx / horizontalDistance) * (WALKER_SPEED * 0.3F);
          }

          if (transform.x < SCREEN_LEFT_BOUNDARY) {
            transform.x = SCREEN_LEFT_BOUNDARY;
            velocity.dx = std::max(0.0F, velocity.dx);
          } else if (transform.x > SCREEN_RIGHT_BOUNDARY) {
            transform.x = SCREEN_RIGHT_BOUNDARY;
            velocity.dx = std::min(0.0F, velocity.dx);
          }

          if (world.hasComponent<ecs::Sprite>(entity)) {
            auto &sprite = world.getComponent<ecs::Sprite>(entity);
            if (sprite.spriteId == ecs::SpriteId::ENEMY_WALKER) {
              if (velocity.dx > -0.1F) {
                sprite.startFrame = 3;
                sprite.endFrame = 5;
              } else if (velocity.dx < 0.1F) {
                sprite.startFrame = 0;
                sprite.endFrame = 2;
              } else {
                sprite.startFrame = 2;
                sprite.endFrame = 2;
              }
            }
          }

          pattern.phase += deltaTime;
          constexpr float SHOOT_INTERVAL = 2.0F;
          float dy = playerPos.y - transform.y;
          float fullDistance = std::sqrt(dx * dx + dy * dy);

          if (pattern.phase >= SHOOT_INTERVAL && horizontalDistance <= SHOOTING_RANGE_MAX &&
              horizontalDistance >= SHOOTING_RANGE_MIN) {
            pattern.phase = 0.0F;
            if (fullDistance > 0.0F) {
              // Copy values before spawning
              float walkerX = transform.x;
              float walkerY = transform.y;
              float targetX = playerPos.x;
              float targetY = playerPos.y;

              constexpr float PROJECTILE_SPEED = 400.0F;
              float dirX = ((targetX - walkerX) / fullDistance) * PROJECTILE_SPEED;
              float dirY = ((targetY - walkerY) / fullDistance) * PROJECTILE_SPEED;

              ecs::Entity projectile = world.createEntity();
              ecs::Transform projTransform;
              projTransform.x = walkerX;
              projTransform.y = walkerY;
              projTransform.rotation = 0.0F;
              projTransform.scale = 0.5F;
              world.addComponent(projectile, projTransform);

              ecs::Velocity projVelocity;
              projVelocity.dx = dirX;
              projVelocity.dy = dirY;
              world.addComponent(projectile, projVelocity);

              ecs::Sprite projSprite;
              projSprite.spriteId = ecs::SpriteId::WALKER_PROJECTILE;
              projSprite.width = 78;
              projSprite.height = 72;
              projSprite.animated = true;
              projSprite.frameCount = 4;
              projSprite.currentFrame = 0;
              projSprite.startFrame = 0;
              projSprite.endFrame = 3;
              projSprite.frameTime = 0.08F;
              projSprite.animationTimer = 0.0F;
              projSprite.reverseAnimation = false;
              projSprite.loop = false;
              world.addComponent(projectile, projSprite);

              ecs::Collider projCollider;
              projCollider.width = 78.0F * 0.5F;
              projCollider.height = 72.0F * 0.5F;
              projCollider.shape = ecs::Collider::Shape::BOX;
              world.addComponent(projectile, projCollider);

              ecs::Owner projOwner;
              projOwner.ownerId = entity;
              world.addComponent(projectile, projOwner);

              ecs::Networked net;
              net.networkId = projectile;
              world.addComponent(projectile, net);
            }
          }
        } else {
          velocity.dx = 0.0F;
          velocity.dy = 0.0F;
        }
      } else if (pattern.patternType == "elite_track") {
        // Elite green enemy: track player at a small distance and fire straight shots
        constexpr float FOLLOW_DISTANCE_DEFAULT = 240.0F;
        constexpr float FOLLOW_SPEED_DEFAULT = 220.0F;
        constexpr float SHOOT_INTERVAL = 1.8F;
        constexpr float SHOOT_FRAME_DURATION = 0.2F;
        constexpr float PROJECTILE_SPEED = 520.0F;

        std::vector<ecs::Entity> players;
        ecs::ComponentSignature playerSig;
        playerSig.set(ecs::getComponentId<ecs::PlayerId>());
        world.getEntitiesWithSignature(playerSig, players);

        if (!players.empty()) {
          auto &playerPos = world.getComponent<ecs::Transform>(players[0]);

          const float followDistance = (pattern.amplitude > 0.0F) ? pattern.amplitude : FOLLOW_DISTANCE_DEFAULT;
          const float followSpeed = (pattern.frequency > 0.0F) ? pattern.frequency : FOLLOW_SPEED_DEFAULT;

          const float desiredX = playerPos.x + followDistance;
          const float desiredY = playerPos.y;

          const float dx = desiredX - transform.x;
          const float dy = desiredY - transform.y;

          const auto clampSpeed = [followSpeed](float value) {
            if (value > followSpeed)
              return followSpeed;
            if (value < -followSpeed)
              return -followSpeed;
            return value;
          };

          velocity.dx = clampSpeed(dx);
          velocity.dy = clampSpeed(dy);

          // Shooting timer using pattern.phase
          pattern.phase += deltaTime;
          bool fired = false;

          if (pattern.phase >= SHOOT_INTERVAL) {
            pattern.phase = 0.0F;
            fired = true;

            // Compute straight direction towards player at fire time
            float shotDx = playerPos.x - transform.x;
            float shotDy = playerPos.y - transform.y;
            float shotDist = std::sqrt(shotDx * shotDx + shotDy * shotDy);
            if (shotDist < 1.0F)
              shotDist = 1.0F;
            const float dirX = (shotDx / shotDist) * PROJECTILE_SPEED;
            const float dirY = (shotDy / shotDist) * PROJECTILE_SPEED;

            constexpr float ELITE_SPRITE_HEIGHT = 58.0F;
            const float enemyHeight = ELITE_SPRITE_HEIGHT * transform.scale;

            const float projectileScale = transform.scale;
            const float projectileWidth = 65.0F * projectileScale;
            const float projectileHeight = 18.0F * projectileScale;

            // Spawn slightly forward (left) and a bit lower on the enemy body
            const float muzzleOffsetX = -projectileWidth * 1.2F;
            const float muzzleOffsetY = (enemyHeight - projectileHeight) * 0.6F;

            // Spawn projectile (elite_enemy_green_out)
            ecs::Entity projectile = world.createEntity();

            ecs::Transform projTransform;
            projTransform.x = transform.x + muzzleOffsetX;
            projTransform.y = transform.y + muzzleOffsetY;
            projTransform.rotation = 0.0F;
            projTransform.scale = projectileScale;
            world.addComponent(projectile, projTransform);

            ecs::Velocity projVelocity;
            projVelocity.dx = dirX;
            projVelocity.dy = dirY;
            world.addComponent(projectile, projVelocity);

            ecs::Sprite projSprite;
            projSprite.spriteId = ecs::SpriteId::ELITE_ENEMY_GREEN_OUT;
            projSprite.width = 65; // elite_enemy_green_out frame width (131/2)
            projSprite.height = 18;
            projSprite.animated = true;
            projSprite.frameCount = 2;
            projSprite.currentFrame = 0;
            projSprite.startFrame = 0;
            projSprite.endFrame = 1;
            projSprite.frameTime = 0.08F;
            projSprite.reverseAnimation = false;
            projSprite.loop = true;
            world.addComponent(projectile, projSprite);

            ecs::Collider projCollider;
            projCollider.width = projSprite.width * projectileScale;
            projCollider.height = projSprite.height * projectileScale;
            projCollider.shape = ecs::Collider::Shape::BOX;
            world.addComponent(projectile, projCollider);

            ecs::Owner projOwner;
            projOwner.ownerId = entity;
            world.addComponent(projectile, projOwner);

            ecs::Networked net;
            net.networkId = projectile;
            world.addComponent(projectile, net);

            // Spawn muzzle flash (elite_enemy_green_in) - one-shot animation
            ecs::Entity muzzle = world.createEntity();

            ecs::Transform muzzleTransform;
            const float muzzleScale = projectileScale; // match projectile height
            const float muzzleWidth = 31.0F * muzzleScale;
            muzzleTransform.x = transform.x - muzzleWidth * 1.2F;
            // Align muzzle Y with projectile Y so both are on the same line
            muzzleTransform.y = projTransform.y;
            muzzleTransform.rotation = 0.0F;
            muzzleTransform.scale = muzzleScale;
            world.addComponent(muzzle, muzzleTransform);

            ecs::Velocity muzzleVelocity;
            muzzleVelocity.dx = 0.0F;
            muzzleVelocity.dy = 0.0F;
            world.addComponent(muzzle, muzzleVelocity);

            ecs::Sprite muzzleSprite;
            muzzleSprite.spriteId = ecs::SpriteId::ELITE_ENEMY_GREEN_IN;
            muzzleSprite.width = 31; // elite_enemy_green_in frame width (93/3)
            muzzleSprite.height = 18;
            muzzleSprite.animated = true;
            muzzleSprite.frameCount = 3;
            muzzleSprite.currentFrame = 0;
            muzzleSprite.startFrame = 0;
            muzzleSprite.endFrame = 2;
            muzzleSprite.frameTime = 0.06F;
            muzzleSprite.reverseAnimation = false;
            muzzleSprite.loop = false;
            world.addComponent(muzzle, muzzleSprite);

            ecs::Lifetime life;
            life.remaining = muzzleSprite.frameTime * static_cast<float>(muzzleSprite.frameCount);
            world.addComponent(muzzle, life);

            ecs::Networked muzzleNet;
            muzzleNet.networkId = muzzle;
            world.addComponent(muzzle, muzzleNet);
          }

          // Update elite green sprite frame based on movement/shooting
          if (world.hasComponent<ecs::Sprite>(entity)) {
            auto &sprite = world.getComponent<ecs::Sprite>(entity);
            if (sprite.spriteId == ecs::SpriteId::ELITE_ENEMY_GREEN) {
              uint32_t frame = 0;
              if (pattern.phase <= SHOOT_FRAME_DURATION || fired) {
                frame = 0; // shooting
              } else if (velocity.dy < -0.1F) {
                frame = 1; // moving up
              } else if (velocity.dy > 0.1F) {
                frame = 2; // moving down
              } else {
                frame = 1;
              }
              sprite.startFrame = frame;
              sprite.endFrame = frame;
              sprite.currentFrame = frame;
            }
          }
        } else {
          // No player found: move left slowly
          velocity.dx = ENEMY_MOVE_SPEED;
          velocity.dy = 0.0F;
        }
      } else if (pattern.patternType == "boss_pattern") {
        // ... (Boss Dobkeratops logic - mostly existing) ...
        constexpr float SCREEN_TOP_BOUNDARY = 0.0F;
        constexpr float SCREEN_BOTTOM_BOUNDARY = 1080.0F;
        constexpr float SCREEN_RIGHT_BOUNDARY = 1920.0F;
        constexpr float DEFAULT_ENTRY_MARGIN = 400.0F;

        struct BossState {
          bool verticalMode = false;
          float speedChangeTimer = 0.0F;
          float nextChangeInterval = 1.0F;
          float targetSpeed = 150.0F;
        };

        static std::unordered_map<ecs::Entity, BossState> s_bossStates;
        auto &state = s_bossStates[entity];

        if (pattern.phase == 0.0F) {
          pattern.phase = 1.0F;
          std::mt19937 rng(static_cast<unsigned int>(entity));
          std::uniform_int_distribution<int> dist(0, 1);
          velocity.dy = (dist(rng) == 0 ? std::abs(velocity.dy) : -std::abs(velocity.dy));
          std::uniform_real_distribution<float> ivar(0.8F, 2.0F);
          std::uniform_real_distribution<float> speedVar(100.0F, 280.0F);
          state.nextChangeInterval = ivar(rng);
          state.targetSpeed = speedVar(rng);
          state.speedChangeTimer = 0.0F;
        }

        float entryX = SCREEN_RIGHT_BOUNDARY - DEFAULT_ENTRY_MARGIN;
        if (world.hasComponent<ecs::Sprite>(entity)) {
          auto &sprite = world.getComponent<ecs::Sprite>(entity);
          float halfWidth = (sprite.width * (world.getComponent<ecs::Transform>(entity).scale));
          entryX = SCREEN_RIGHT_BOUNDARY - halfWidth;
        }

        if (!state.verticalMode) {
          if (transform.x <= entryX) {
            state.verticalMode = true;
            transform.x = entryX;
            velocity.dx = 0.0F;
            float sign = (velocity.dy < 0.0F ? -1.0F : 1.0F);
            velocity.dy = sign * state.targetSpeed;
            transform.y = std::clamp(transform.y, SCREEN_TOP_BOUNDARY + 20.0F, SCREEN_BOTTOM_BOUNDARY - 20.0F);
          }
        } else {
          transform.x = entryX;
          velocity.dx = 0.0F;
          state.speedChangeTimer += deltaTime;
          if (state.speedChangeTimer >= state.nextChangeInterval) {
            std::mt19937 rng(static_cast<unsigned int>(entity) +
                             static_cast<unsigned int>(state.speedChangeTimer * 1000.0F));
            std::uniform_real_distribution<float> ivar(0.6F, 2.2F);
            std::uniform_real_distribution<float> speedVar(90.0F, 340.0F);
            state.nextChangeInterval = ivar(rng);
            state.targetSpeed = speedVar(rng);
            state.speedChangeTimer = 0.0F;
          }
          constexpr float SPEED_LERP = 4.0F;
          float curSpeed = std::abs(velocity.dy);
          float newSpeed = curSpeed + (state.targetSpeed - curSpeed) * std::min(1.0F, SPEED_LERP * deltaTime);
          velocity.dy = (velocity.dy < 0.0F ? -newSpeed : newSpeed);

          if (transform.y <= SCREEN_TOP_BOUNDARY + 10.0F && velocity.dy < 0.0F)
            velocity.dy = -velocity.dy;
          if (transform.y >= SCREEN_BOTTOM_BOUNDARY - 10.0F && velocity.dy > 0.0F)
            velocity.dy = -velocity.dy;
          transform.y = std::clamp(transform.y, SCREEN_TOP_BOUNDARY + 1.0F, SCREEN_BOTTOM_BOUNDARY - 1.0F);
        }

        pattern.amplitude += deltaTime;
        constexpr float ROBOT_SHOOT_INTERVAL = 2.5F;
        if (pattern.amplitude >= ROBOT_SHOOT_INTERVAL) {
          pattern.amplitude = 0.0F;
          std::vector<ecs::Entity> players;
          ecs::ComponentSignature playerSig;
          playerSig.set(ecs::getComponentId<ecs::PlayerId>());
          world.getEntitiesWithSignature(playerSig, players);
          if (!players.empty()) {
            float bossX = transform.x;
            float bossY = transform.y;
            auto &playerPos = world.getComponent<ecs::Transform>(players[0]);
            float targetX = playerPos.x;
            float targetY = playerPos.y;

            float dx = targetX - bossX;
            float dy = targetY - bossY;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance > 0.0F) {
              constexpr float ROBOT_PROJECTILE_SPEED = 350.0F;
              float dirX = (dx / distance) * ROBOT_PROJECTILE_SPEED;
              float dirY = (dy / distance) * ROBOT_PROJECTILE_SPEED;

              ecs::Entity projectile = world.createEntity();
              ecs::Transform projTransform;
              projTransform.x = bossX;
              projTransform.y = bossY;
              projTransform.rotation = 1.0F;
              projTransform.scale = 3.0F;
              world.addComponent(projectile, projTransform);

              ecs::Velocity projVelocity;
              projVelocity.dx = dirX;
              projVelocity.dy = dirY;
              world.addComponent(projectile, projVelocity);

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

              ecs::Collider projCollider;
              projCollider.width = 34.0F;
              projCollider.height = 34.0F;
              projCollider.shape = ecs::Collider::Shape::CIRCLE;
              world.addComponent(projectile, projCollider);

              ecs::Owner projOwner;
              projOwner.ownerId = entity;
              world.addComponent(projectile, projOwner);

              ecs::Attraction projAttraction;
              projAttraction.force = 500.0F;
              projAttraction.radius = 300.0F;
              world.addComponent(projectile, projAttraction);

              ecs::Networked net;
              net.networkId = projectile;
              world.addComponent(projectile, net);
            }
          }
        }

      } else if (pattern.patternType == "boss_brocolis_pattern") {
        // ... (Boss Brocolis logic) ...
        struct BrocolisState {
          bool hasEntered = false;
          bool isHatching = false;
          float hatchingTimer = 0.0F;
        };
        static std::unordered_map<ecs::Entity, BrocolisState> s_brocolisStates;
        auto &state = s_brocolisStates[entity];

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
          auto &sprite = world.getComponent<ecs::Sprite>(entity);
          if (isProjectile && !state.isHatching) {
            if (world.hasComponent<ecs::Health>(entity)) {
              const auto &hp = world.getComponent<ecs::Health>(entity);
              if (hp.hp < hp.maxHp) {
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
                if (ownerIsParentBoss) {
                  state.isHatching = true;
                  sprite.spriteId = ecs::SpriteId::BOSS_BROCOLIS_ECLOSION;
                  velocity.dx = 0.0F;
                  velocity.dy = 0.0F;
                  sprite.animated = true;
                  sprite.reverseAnimation = true;
                  sprite.startFrame = 0;
                  sprite.endFrame = 3;
                  sprite.currentFrame = 3;
                  sprite.loop = false;
                  sprite.frameTime = 0.15F;
                  state.hatchingTimer = 0.0F;
                }
              }
            }
          } else if (state.isHatching || isHatchingEgg) {
            velocity.dx = 0.0F;
            velocity.dy = 0.0F;
            state.hatchingTimer += deltaTime;
            constexpr float HATCH_DURATION = 0.6F;
            if (state.hatchingTimer >= HATCH_DURATION) {
              float spawnX = transform.x;
              float spawnY = transform.y;

              ecs::Entity newBoss = world.createEntity();
              ecs::Transform bossTrans;
              bossTrans.x = spawnX;
              bossTrans.y = spawnY;
              bossTrans.scale = 1.5F;
              world.addComponent(newBoss, bossTrans);

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

              ecs::Velocity bossVel;
              bossVel.dx = 0.0F;
              bossVel.dy = 0.0F;
              world.addComponent(newBoss, bossVel);

              ecs::Collider bossCol;
              bossCol.width = 33.0F * 1.5F;
              bossCol.height = 34.0F * 1.5F;
              world.addComponent(newBoss, bossCol);

              ecs::Health bossHp;
              bossHp.maxHp = 200;
              bossHp.hp = 200;
              world.addComponent(newBoss, bossHp);

              ecs::Pattern bossPat;
              bossPat.patternType = "boss_brocolis_pattern";
              bossPat.phase = 0.0F;
              world.addComponent(newBoss, bossPat);

              ecs::Networked net;
              net.networkId = newBoss;
              world.addComponent(newBoss, net);

              world.destroyEntity(entity);
              s_brocolisStates.erase(entity);
            }
          }
        } else {
          constexpr float TARGET_ENTER_X = 960.0F;
          constexpr float TARGET_ENTER_Y = 200.0F;
          constexpr float ENTER_SPEED = 300.0F;
          constexpr float ENTER_THRESHOLD = 10.0F;

          if (!state.hasEntered) {
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
            constexpr float PREFERRED_DISTANCE = 600.0F;
            constexpr float MOVE_SPEED = 200.0F;
            constexpr float SHOOT_INTERVAL = 5.0F;
            constexpr float SCREEN_MARGIN = 50.0F;

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

            if (transform.scale > 2.0F) {
              pattern.phase += deltaTime;
              if (pattern.phase >= SHOOT_INTERVAL) {
                pattern.phase = 0.0F;
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

                // Copy values
                float bossX = transform.x;
                float bossY = transform.y;

                ecs::Entity proj = world.createEntity();
                ecs::Transform projTrans;
                projTrans.x = bossX;
                projTrans.y = bossY + 40.0F;
                projTrans.scale = 0.75F;
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
                projSprite.animationTimer = 0.0F;
                projSprite.reverseAnimation = false;
                projSprite.loop = true;
                world.addComponent(proj, projSprite);

                constexpr float PROJ_SPEED = 300.0F;
                ecs::Velocity projVel;
                projVel.dx = shootDirX * PROJ_SPEED;
                projVel.dy = shootDirY * PROJ_SPEED;
                world.addComponent(proj, projVel);

                ecs::Pattern projPat;
                projPat.patternType = "boss_brocolis_pattern";
                world.addComponent(proj, projPat);

                ecs::Health projHp;
                projHp.maxHp = 10;
                projHp.hp = 10;
                world.addComponent(proj, projHp);

                ecs::Collider projCol;
                projCol.width = 33.0F * 0.75F;
                projCol.height = 31.0F * 0.75F;
                projCol.shape = ecs::Collider::Shape::CIRCLE;
                world.addComponent(proj, projCol);

                ecs::Owner owner;
                owner.ownerId = entity;
                world.addComponent(proj, owner);

                ecs::Networked net;
                net.networkId = proj;
                world.addComponent(proj, net);
              }
            } else if (transform.scale > 1.0F) {
              constexpr float MINI_SHOOT_INTERVAL = 3.0F;
              pattern.phase += deltaTime;
              if (pattern.phase >= MINI_SHOOT_INTERVAL) {
                pattern.phase = 0.0F;
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

                // Copy values
                float bossX = transform.x;
                float bossY = transform.y;

                ecs::Entity proj = world.createEntity();
                ecs::Transform projTrans;
                projTrans.x = bossX;
                projTrans.y = bossY + 28.0F;
                projTrans.scale = 0.65F;
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

                constexpr float PROJ_SPEED_CHILD = 240.0F;
                ecs::Velocity projVel;
                projVel.dx = shootDirX * PROJ_SPEED_CHILD;
                projVel.dy = shootDirY * PROJ_SPEED_CHILD;
                world.addComponent(proj, projVel);

                ecs::Pattern projPat;
                projPat.patternType = "boss_brocolis_pattern";
                world.addComponent(proj, projPat);

                ecs::Health projHp;
                projHp.maxHp = 6;
                projHp.hp = 6;
                world.addComponent(proj, projHp);

                ecs::Collider projCol;
                projCol.width = 28.0F * 0.65F;
                projCol.height = 28.0F * 0.65F;
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

          if (bState.timer == 0.0F && bState.spawnX == 0.0F && bState.spawnY == 0.0F) {
            bState.spawnX = transform.x;
            bState.spawnY = transform.y;
          }

          bState.timer += deltaTime;

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
              if (velocity.dx == 0 && velocity.dy == 0)
                velocity.dx = -PROJ_SPEED;
            }
          } else if (bState.timer >= BOOMERANG_TIMER && !bState.hasReachedSpawn) {
            bState.returning = true;
            float dx = bState.spawnX - transform.x;
            float dy = bState.spawnY - transform.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 20.0F) {
              bState.hasReachedSpawn = true;
              bState.timer = 0.0F;
            } else if (dist > 0.0F) {
              velocity.dx = (dx / dist) * PROJ_SPEED;
              velocity.dy = (dy / dist) * PROJ_SPEED;
              float angleRad = std::atan2(velocity.dy, velocity.dx);
              transform.rotation = angleRad * (180.0F / 3.14159F);
            }
          } else if (bState.hasReachedSpawn) {
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

          if (transform.x < -400.0F || transform.x > 2320.0F || transform.y < -400.0F || transform.y > 1480.0F) {
            world.destroyEntity(entity);
            s_boomerangStates.erase(entity);
          }

        } else {
          // BOSS EVANGELIC LOGIC
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

            if (!players.empty() && currentProjectiles < MAX_PROJECTILES) {
              // SAFE COPY: Capture player position values before any addComponent call
              auto &playerTrans = world.getComponent<ecs::Transform>(players[0]);
              float targetX = playerTrans.x;
              float targetY = playerTrans.y;

              // SAFE COPY: Capture boss position
              float bossX = transform.x;

              int toSpawn = std::min(2, MAX_PROJECTILES - currentProjectiles);

              for (int side = 0; side < toSpawn; ++side) {
                ecs::Entity proj = world.createEntity();

                ecs::Transform projTrans;
                projTrans.x = bossX; // Use copy
                projTrans.y = (side == 0) ? EDGE_MARGIN : (1080.0F - EDGE_MARGIN);
                projTrans.rotation = 0.0F;
                projTrans.scale = 3.0F;
                world.addComponent(proj, projTrans);

                // Use copies for calculation
                float dx = targetX - projTrans.x;
                float dy = targetY - projTrans.y;
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
                projCol.width = 32.0F * 3.0F;
                projCol.height = 30.0F * 3.0F;
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
      }

      // Update rotation for yellow bee based on velocity direction
      if (world.hasComponent<ecs::Sprite>(entity)) {
        auto &sprite = world.getComponent<ecs::Sprite>(entity);
        if (sprite.spriteId == ecs::SpriteId::ENEMY_YELLOW) {
          // Re-fetch velocity/transform just in case, though ENEMY_YELLOW doesn't trigger spawns
          if (world.hasComponent<ecs::Velocity>(entity) && world.hasComponent<ecs::Transform>(entity)) {
            auto &currVel = world.getComponent<ecs::Velocity>(entity);
            auto &currTrans = world.getComponent<ecs::Transform>(entity);

            float targetAngle = std::atan2(currVel.dy, currVel.dx) * (180.0F / 3.14159265F) + 180.0F;
            float currentAngle = currTrans.rotation;
            float angleDiff = targetAngle - currentAngle;
            while (angleDiff > 180.0F)
              angleDiff -= 360.0F;
            while (angleDiff < -180.0F)
              angleDiff += 360.0F;
            constexpr float ROTATION_SPEED = 180.0F;
            float maxRotation = ROTATION_SPEED * deltaTime;
            if (std::abs(angleDiff) < maxRotation) {
              currTrans.rotation = targetAngle;
            } else {
              currTrans.rotation += (angleDiff > 0 ? maxRotation : -maxRotation);
            }
            while (currTrans.rotation >= 360.0F)
              currTrans.rotation -= 360.0F;
            while (currTrans.rotation < 0.0F)
              currTrans.rotation += 360.0F;
          }
        }
      }

      // Safe destruction check at end of loop
      // Refetch components because previous references (transform, pattern) may be invalid due to vector reallocation
      if (world.isAlive(entity) && world.hasComponent<ecs::Transform>(entity) &&
          world.hasComponent<ecs::Pattern>(entity)) {
        const auto &currTrans = world.getComponent<ecs::Transform>(entity);
        const auto &currPattern = world.getComponent<ecs::Pattern>(entity);

        if (currPattern.patternType != "ground_walk" && currTrans.x < OFFSCREEN_DESTROY_X) {
          world.destroyEntity(entity);
        }
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
  ecs::EventListenerHandle m_damageHandle;
  static constexpr float ENEMY_MOVE_SPEED = -384.0F;
  static constexpr float OFFSCREEN_DESTROY_X = -100.0F;
};

} // namespace server

#endif // SERVER_ENEMY_AI_SYSTEM_HPP_