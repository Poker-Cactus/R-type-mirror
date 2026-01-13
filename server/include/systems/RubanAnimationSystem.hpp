/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** RubanAnimationSystem.hpp - Handles R-Type ribbon beam sprite transitions
*/

#ifndef SERVER_RUBAN_ANIMATION_SYSTEM_HPP_
#define SERVER_RUBAN_ANIMATION_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "ecs/ComponentSignature.hpp"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace server
{

/**
 * @brief Configuration for each ruban sprite phase
 * Sprites use xruban_projectile.png format where x is the phase number (1-14)
 */
struct RubanPhaseConfig {
  std::uint32_t spriteId;
  std::uint32_t width; // Full spritesheet width
  std::uint32_t height; // Sprite height
  std::uint32_t frameCount; // Number of frames (1 for single images)
  std::uint32_t frameWidth; // Width per frame
};

// Phase configurations based on actual sprite dimensions from ruban_projectile_sprite folder
// All sprites are single-frame images
// Sprites 1-16 are transition, 17-24 loop continuously
static constexpr std::array<RubanPhaseConfig, 24> RUBAN_PHASES = {{// Phase 1: 21x49, 1 frame
                                                                   {ecs::SpriteId::RUBAN1_PROJECTILE, 21, 49, 1, 21},
                                                                   // Phase 2: 28x55, 1 frame
                                                                   {ecs::SpriteId::RUBAN2_PROJECTILE, 28, 55, 1, 28},
                                                                   // Phase 3: 36x66, 1 frame
                                                                   {ecs::SpriteId::RUBAN3_PROJECTILE, 36, 66, 1, 36},
                                                                   // Phase 4: 47x66, 1 frame
                                                                   {ecs::SpriteId::RUBAN4_PROJECTILE, 47, 66, 1, 47},
                                                                   // Phase 5: 50x66, 1 frame
                                                                   {ecs::SpriteId::RUBAN5_PROJECTILE, 50, 66, 1, 50},
                                                                   // Phase 6: 59x60, 1 frame
                                                                   {ecs::SpriteId::RUBAN6_PROJECTILE, 59, 60, 1, 59},
                                                                   // Phase 7: 70x62, 1 frame
                                                                   {ecs::SpriteId::RUBAN7_PROJECTILE, 70, 62, 1, 70},
                                                                   // Phase 8: 66x64, 1 frame
                                                                   {ecs::SpriteId::RUBAN8_PROJECTILE, 66, 64, 1, 66},
                                                                   // Phase 9: 65x72, 1 frame
                                                                   {ecs::SpriteId::RUBAN9_PROJECTILE, 65, 72, 1, 65},
                                                                   // Phase 10: 67x75, 1 frame
                                                                   {ecs::SpriteId::RUBAN10_PROJECTILE, 67, 75, 1, 67},
                                                                   // Phase 11: 65x64, 1 frame
                                                                   {ecs::SpriteId::RUBAN11_PROJECTILE, 65, 64, 1, 65},
                                                                   // Phase 12: 64x65, 1 frame
                                                                   {ecs::SpriteId::RUBAN12_PROJECTILE, 64, 65, 1, 64},
                                                                   // Phase 13: 63x63, 1 frame
                                                                   {ecs::SpriteId::RUBAN13_PROJECTILE, 63, 63, 1, 63},
                                                                   // Phase 14: 66x60, 1 frame
                                                                   {ecs::SpriteId::RUBAN14_PROJECTILE, 66, 60, 1, 66},
                                                                   // Phase 15: 72x57, 1 frame
                                                                   {ecs::SpriteId::RUBAN15_PROJECTILE, 72, 57, 1, 72},
                                                                   // Phase 16: 66x38, 1 frame
                                                                   {ecs::SpriteId::RUBAN16_PROJECTILE, 66, 38, 1, 66},
                                                                   // Phase 17: 66x36, 1 frame (loop start)
                                                                   {ecs::SpriteId::RUBAN17_PROJECTILE, 66, 36, 1, 66},
                                                                   // Phase 18: 63x37, 1 frame
                                                                   {ecs::SpriteId::RUBAN18_PROJECTILE, 63, 37, 1, 63},
                                                                   // Phase 19: 64x35, 1 frame
                                                                   {ecs::SpriteId::RUBAN19_PROJECTILE, 64, 35, 1, 64},
                                                                   // Phase 20: 65x34, 1 frame
                                                                   {ecs::SpriteId::RUBAN20_PROJECTILE, 65, 34, 1, 65},
                                                                   // Phase 21: 67x35, 1 frame
                                                                   {ecs::SpriteId::RUBAN21_PROJECTILE, 67, 35, 1, 67},
                                                                   // Phase 22: 63x34, 1 frame
                                                                   {ecs::SpriteId::RUBAN22_PROJECTILE, 63, 34, 1, 63},
                                                                   // Phase 23: 62x34, 1 frame
                                                                   {ecs::SpriteId::RUBAN23_PROJECTILE, 62, 34, 1, 62},
                                                                   // Phase 24: 67x34, 1 frame
                                                                   {ecs::SpriteId::RUBAN24_PROJECTILE, 67, 34, 1, 67}}};

/**
 * @brief System that manages R-Type ribbon beam sprite transitions
 *
 * The ribbon projectile cycles through sprites 1→2→...→16 (transition phases)
 * Then loops continuously through sprites 17→18→...→24→17...
 * All sprites are single-frame images from ruban_projectile_sprite folder
 */

// Reference dimensions for centering (use largest sprite as reference)
static constexpr std::uint32_t RUBAN_REF_WIDTH = 72; // Max width among all sprites
static constexpr std::uint32_t RUBAN_REF_HEIGHT = 75; // Max height among all sprites

struct RubanAnimationData {
  std::uint32_t currentPhase = 1;
  float phaseTimer = 0.0f;
  float phaseDuration = 0.025f; // 25ms per phase transition (faster animation)
  bool completed = false;
  float lastOffsetX = 0.0f; // Track last applied offset for correction
  float lastOffsetY = 0.0f;
};

class RubanAnimationSystem : public ecs::ISystem
{
private:
  std::unordered_map<ecs::Entity, RubanAnimationData> _animations;

public:
  void update(ecs::World &world, float deltaTime) override
  {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);
    std::unordered_set<ecs::Entity> processedEntities;

    for (auto entity : entities) {
      auto &sprite = world.getComponent<ecs::Sprite>(entity);

      if (_animations.find(entity) == _animations.end()) {
        if (sprite.spriteId == RUBAN_PHASES[0].spriteId) {
          _animations[entity] = RubanAnimationData();
        } else {
          continue;
        }
      }

      processedEntities.insert(entity);
      auto &rubanAnim = _animations[entity];

      // completed = reached the loop phase (17-24)
      if (rubanAnim.completed) {
        rubanAnim.phaseTimer += deltaTime;
        if (rubanAnim.phaseTimer >= rubanAnim.phaseDuration) {
          rubanAnim.phaseTimer = 0.0f;

          // Get previous phase config for offset correction
          std::uint32_t prevIdx = rubanAnim.currentPhase - 1;
          const auto &prevConfig = RUBAN_PHASES[prevIdx];

          rubanAnim.currentPhase++;

          // Loop 17 -> 18 -> ... -> 24 -> 17
          if (rubanAnim.currentPhase > 24) {
            rubanAnim.currentPhase = 17;
          }

          // Update sprite for the loop phase
          std::uint32_t idx = rubanAnim.currentPhase - 1;
          const auto &config = RUBAN_PHASES[idx];
          sprite.spriteId = config.spriteId;
          sprite.width = config.frameWidth;
          sprite.height = config.height;
          sprite.frameCount = config.frameCount;
          sprite.currentFrame = 0;
          sprite.animated = false;

          // Apply centering offset correction
          if (world.hasComponent<ecs::Transform>(entity)) {
            auto &transform = world.getComponent<ecs::Transform>(entity);
            // Calculate offset difference between sprites to keep them centered
            float offsetX = (static_cast<float>(prevConfig.frameWidth) - static_cast<float>(config.frameWidth)) * 0.5f *
              3.0f; // 3.0f = RUBAN_SCALE
            float offsetY = (static_cast<float>(prevConfig.height) - static_cast<float>(config.height)) * 0.5f * 3.0f;
            transform.x += offsetX;
            transform.y += offsetY;
          }
        }
        continue;
      }

      // Update phase timer
      rubanAnim.phaseTimer += deltaTime;

      // Check if we should transition to next phase
      if (rubanAnim.phaseTimer >= rubanAnim.phaseDuration) {
        rubanAnim.phaseTimer = 0.0f;

        // Get previous phase config for offset correction
        std::uint32_t prevIdx = rubanAnim.currentPhase - 1;
        const auto &prevConfig = RUBAN_PHASES[prevIdx];

        rubanAnim.currentPhase++;

        // Enter loop phase (>= 17)
        if (rubanAnim.currentPhase >= 17) {
          rubanAnim.phaseDuration = 0.04f; // Fast loop animation
          rubanAnim.currentPhase = 17;
          rubanAnim.completed = true;

          const auto &config = RUBAN_PHASES[16]; // Phase 17 is index 16
          sprite.spriteId = config.spriteId;
          sprite.width = config.frameWidth;
          sprite.height = config.height;
          sprite.frameCount = config.frameCount;
          sprite.currentFrame = 0;
          sprite.animated = false;
          sprite.loop = false;

          // Apply centering offset correction
          if (world.hasComponent<ecs::Transform>(entity)) {
            auto &transform = world.getComponent<ecs::Transform>(entity);
            float offsetX =
              (static_cast<float>(prevConfig.frameWidth) - static_cast<float>(config.frameWidth)) * 0.5f * 3.0f;
            float offsetY = (static_cast<float>(prevConfig.height) - static_cast<float>(config.height)) * 0.5f * 3.0f;
            transform.x += offsetX;
            transform.y += offsetY;
          }
        } else {
          // Transition phases (1-16)
          std::uint32_t idx = rubanAnim.currentPhase - 1;
          if (idx < RUBAN_PHASES.size()) {
            const auto &config = RUBAN_PHASES[idx];
            sprite.spriteId = config.spriteId;
            sprite.width = config.frameWidth;
            sprite.height = config.height;
            sprite.frameCount = config.frameCount;
            sprite.currentFrame = 0;
            sprite.animated = false;

            // Apply centering offset correction during transitions
            if (world.hasComponent<ecs::Transform>(entity)) {
              auto &transform = world.getComponent<ecs::Transform>(entity);
              float offsetX =
                (static_cast<float>(prevConfig.frameWidth) - static_cast<float>(config.frameWidth)) * 0.5f * 3.0f;
              float offsetY = (static_cast<float>(prevConfig.height) - static_cast<float>(config.height)) * 0.5f * 3.0f;
              transform.x += offsetX;
              transform.y += offsetY;
            }
          }
        }
      }
    }

    // Cleanup animations for entities that no longer exist
    for (auto it = _animations.begin(); it != _animations.end();) {
      if (processedEntities.find(it->first) == processedEntities.end()) {
        it = _animations.erase(it);
      } else {
        ++it;
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Sprite>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    return sig;
  }
};

} // namespace server

#endif // SERVER_RUBAN_ANIMATION_SYSTEM_HPP_
