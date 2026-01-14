/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ShootingSystem.hpp - Handles shooting logic
*/

#ifndef SERVER_SHOOTING_SYSTEM_HPP_
#define SERVER_SHOOTING_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Charging.hpp"
#include "../../../engineCore/include/ecs/components/Follower.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Owner.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

namespace server
{

/**
 * @brief System that handles shooting when input is pressed
 */
class ShootingSystem : public ecs::ISystem
{
public:
  ShootingSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    m_currentTime += deltaTime;

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      const auto &input = world.getComponent<ecs::Input>(entity);

      // Tir normal
      const bool wasShooting = m_prevShootState.contains(entity) ? m_prevShootState[entity] : false;
      const bool justPressed = input.shoot && !wasShooting;

      if (justPressed && canShoot(entity, world)) {
        // Emit shoot event
        ecs::ShootEvent shootEvent(entity, 1.0F, 0.0F);
        world.emitEvent(shootEvent);
        m_lastShootTime[entity] = m_currentTime;
      }

      // Gestion du tir chargé avec animation automatique
      const bool wasChargedShooting =
        m_prevChargedShootState.contains(entity) ? m_prevChargedShootState[entity] : false;
      const bool justChargedPressed = input.chargedShoot && !wasChargedShooting;

      // Add Charging component if entity doesn't have one
      if (!world.hasComponent<ecs::Charging>(entity)) {
        world.addComponent(entity, ecs::Charging{});
      }
      auto &charging = world.getComponent<ecs::Charging>(entity);

      // Start charging when key is pressed (one-shot trigger)
      if (justChargedPressed && canChargedShoot(entity) && !charging.isCharging) {
        constexpr float LOADING_OFFSET_X = 130.0F;
        constexpr float LOADING_OFFSET_Y = 0.0F;
        float transformX = world.getComponent<ecs::Transform>(entity).x + LOADING_OFFSET_X;
        float transformY = world.getComponent<ecs::Transform>(entity).y + LOADING_OFFSET_Y;

        // Spawn loading shot animation
        ecs::SpawnEntityEvent loadingEvent(ecs::SpawnEntityEvent::EntityType::LOADING_SHOT, transformX, transformY,
                                           entity);
        world.emitEvent(loadingEvent);

        // Find the loading shot entity
        ecs::Entity loadingShotEntity = 0;
        ecs::ComponentSignature ownerSig;
        ownerSig.set(ecs::getComponentId<ecs::Owner>());
        ownerSig.set(ecs::getComponentId<ecs::Sprite>());
        std::vector<ecs::Entity> ownedEntities;
        world.getEntitiesWithSignature(ownerSig, ownedEntities);

        for (auto owned : ownedEntities) {
          const auto &owner = world.getComponent<ecs::Owner>(owned);
          const auto &sprite = world.getComponent<ecs::Sprite>(owned);
          if (owner.ownerId == entity && sprite.spriteId == ecs::SpriteId::LOADING_SHOT) {
            loadingShotEntity = owned;
            break;
          }
        }

        // Add Follower component to make LOADING_SHOT follow the player
        if (loadingShotEntity != 0 && world.isAlive(loadingShotEntity)) {
          ecs::Follower follower;
          follower.parent = entity;
          follower.offsetX = LOADING_OFFSET_X;
          follower.offsetY = LOADING_OFFSET_Y;
          follower.smoothing = 100.0F; // High smoothing for instant positioning
          world.addComponent(loadingShotEntity, follower);
        }

        // Store charging state in component
        charging.isCharging = true;
        charging.chargeTime = 0.0F;
        charging.maxChargeTime = 1.2F;
        charging.loadingShotEntity = loadingShotEntity;

        std::cout << "[ShootingSystem] Started charging for entity " << entity
                  << " (loading shot: " << loadingShotEntity << ")" << std::endl;
      }

      // Update charge time automatically (no need to hold the key)
      if (charging.isCharging) {
        charging.chargeTime += deltaTime;

        // Auto-fire when charge is complete
        if (charging.chargeTime >= charging.maxChargeTime) {
          constexpr float CHARGED_OFFSET_X = 105.0F;
          constexpr float CHARGED_OFFSET_Y = 25.0F;

          // Ne plus réutiliser la position du LOADING_SHOT
          float spawnX = world.getComponent<ecs::Transform>(entity).x + CHARGED_OFFSET_X;
          float spawnY = world.getComponent<ecs::Transform>(entity).y + CHARGED_OFFSET_Y;

          ecs::SpawnEntityEvent spawnEvent(ecs::SpawnEntityEvent::EntityType::CHARGED_PROJECTILE, spawnX, spawnY,
                                           entity);
          world.emitEvent(spawnEvent);

          if (charging.loadingShotEntity != 0 && world.isAlive(charging.loadingShotEntity)) {
            world.destroyEntity(charging.loadingShotEntity);
          }
          m_lastChargedShootTime[entity] = m_currentTime;
          charging.isCharging = false;
          charging.chargeTime = 0.0F;
          charging.loadingShotEntity = 0;
        }
      }

      m_prevShootState[entity] = input.shoot;
      m_prevChargedShootState[entity] = input.chargedShoot;
    }
  }

  /**
   * @brief Subscribe to shoot events to spawn projectiles
   */
  void initialize(ecs::World &world)
  {
    m_shootHandle =
      world.subscribeEvent<ecs::ShootEvent>([&world](const ecs::ShootEvent &event) { spawnProjectile(world, event); });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Transform>());
    sig.set(ecs::getComponentId<ecs::Input>());
    return sig;
  }

private:
  ecs::EventListenerHandle m_shootHandle;
  std::unordered_map<ecs::Entity, float> m_lastShootTime;
  std::unordered_map<ecs::Entity, float> m_lastChargedShootTime;
  std::unordered_map<ecs::Entity, bool> m_prevShootState;
  float m_currentTime = 0.0F;
  const float SHOOT_COOLDOWN = 0.05F; // 5 shots per second (default)
  const float RUBAN_SHOOT_COOLDOWN = 0.02F; // 50 shots per second for ruban (faster)
  std::unordered_map<ecs::Entity, bool> m_prevChargedShootState;
  const float CHARGED_SHOOT_COOLDOWN = 2.0F; // Cooldown between charged shots

  bool canShoot(ecs::Entity entity, ecs::World &world)
  {
    auto iter = m_lastShootTime.find(entity);
    float cooldown = SHOOT_COOLDOWN;

    // Check if player has ruban bubble for faster shooting
    if (hasRubanBubble(world, entity)) {
      cooldown = RUBAN_SHOOT_COOLDOWN;
    }

    if (iter == m_lastShootTime.end()) {
      return true;
    }
    return (m_currentTime - iter->second) >= cooldown;
  }

  bool canChargedShoot(ecs::Entity entity)
  {
    auto iter = m_lastChargedShootTime.find(entity);
    if (iter == m_lastChargedShootTime.end()) {
      return true;
    }
    return (m_currentTime - iter->second) >= CHARGED_SHOOT_COOLDOWN;
  }

  static bool hasRubanBubble(ecs::World &world, ecs::Entity player)
  {
    // Find all drones/bubbles that follow this player
    ecs::ComponentSignature bubbleSig;
    bubbleSig.set(ecs::getComponentId<ecs::Follower>());
    bubbleSig.set(ecs::getComponentId<ecs::Sprite>());

    std::vector<ecs::Entity> bubbles;
    world.getEntitiesWithSignature(bubbleSig, bubbles);

    for (const auto &bubble : bubbles) {
      if (!world.isAlive(bubble)) {
        continue;
      }

      const auto &follower = world.getComponent<ecs::Follower>(bubble);
      if (follower.parent != player) {
        continue;
      }

      const auto &sprite = world.getComponent<ecs::Sprite>(bubble);
      // Check for all ruban bubble sprite IDs (legacy + new individual frames)
      bool isRubanSprite =
        (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN1 && sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN3) ||
        (sprite.spriteId >= ecs::SpriteId::BUBBLE_RUBAN_BACK1 && sprite.spriteId <= ecs::SpriteId::BUBBLE_RUBAN_FRONT4);
      if (isRubanSprite) {
        return true;
      }
    }
    return false;
  }

  static void spawnProjectile(ecs::World &world, const ecs::ShootEvent &event)
  {
    if (!world.isAlive(event.shooter)) {
      return;
    }

    const auto &transform = world.getComponent<ecs::Transform>(event.shooter);

    const float offsetX = 105.0F;
    const float offsetY = 25.0F;

    // Emit spawn event for projectile from the shooter
    ecs::SpawnEntityEvent spawnEvent(ecs::SpawnEntityEvent::EntityType::PROJECTILE, transform.x + offsetX,
                                     transform.y + offsetY, event.shooter);
    world.emitEvent(spawnEvent);

    // Also make all drones following this player shoot
    spawnDroneProjectiles(world, event.shooter);
  }

  static ecs::SpawnEntityEvent::EntityType whichProjectile(std::uint32_t bubble)
  {
    switch (bubble) {
    case ecs::SpriteId::BUBBLE:
      return ecs::SpawnEntityEvent::EntityType::NONE; // Simple bubble doesn't shoot
    case ecs::SpriteId::DRONE:
      return ecs::SpawnEntityEvent::EntityType::PROJECTILE;
    case ecs::SpriteId::BUBBLE_TRIPLE:
      return ecs::SpawnEntityEvent::EntityType::TRIPLE_PROJECTILE;
    // All ruban bubble sprites fire RUBAN1_PROJECTILE
    case ecs::SpriteId::BUBBLE_RUBAN1:
    case ecs::SpriteId::BUBBLE_RUBAN2:
    case ecs::SpriteId::BUBBLE_RUBAN3:
    case ecs::SpriteId::BUBBLE_RUBAN_BACK1:
    case ecs::SpriteId::BUBBLE_RUBAN_BACK2:
    case ecs::SpriteId::BUBBLE_RUBAN_BACK3:
    case ecs::SpriteId::BUBBLE_RUBAN_BACK4:
    case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE1:
    case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE2:
    case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE3:
    case ecs::SpriteId::BUBBLE_RUBAN_MIDDLE4:
    case ecs::SpriteId::BUBBLE_RUBAN_FRONT1:
    case ecs::SpriteId::BUBBLE_RUBAN_FRONT2:
    case ecs::SpriteId::BUBBLE_RUBAN_FRONT3:
    case ecs::SpriteId::BUBBLE_RUBAN_FRONT4:
      return ecs::SpawnEntityEvent::EntityType::RUBAN1_PROJECTILE;
    default:
      return ecs::SpawnEntityEvent::EntityType::PROJECTILE;
    }
  }

  static void spawnDroneProjectiles(ecs::World &world, ecs::Entity player)
  {
    // Find all drones that follow this player
    ecs::ComponentSignature droneSig;
    droneSig.set(ecs::getComponentId<ecs::Follower>());
    droneSig.set(ecs::getComponentId<ecs::Transform>());

    std::vector<ecs::Entity> drones;
    world.getEntitiesWithSignature(droneSig, drones);

    const float droneOffsetX = 30.0F; // Smaller offset for drone projectiles
    const float droneOffsetY = 10.0F;

    for (const auto &drone : drones) {
      if (!world.isAlive(drone)) {
        continue;
      }

      const auto &follower = world.getComponent<ecs::Follower>(drone);

      // Only shoot if this drone follows the player who shot
      if (follower.parent != player) {
        continue;
      }

      const auto &droneTransform = world.getComponent<ecs::Transform>(drone);

      // Determine projectile type based on follower sprite
      ecs::SpawnEntityEvent::EntityType projectileType = ecs::SpawnEntityEvent::EntityType::PROJECTILE;
      if (world.hasComponent<ecs::Sprite>(drone)) {
        const auto &sprite = world.getComponent<ecs::Sprite>(drone);
        projectileType = whichProjectile(sprite.spriteId);
      }

      // Skip if this bubble type doesn't shoot (simple bubble)
      if (projectileType == ecs::SpawnEntityEvent::EntityType::NONE) {
        continue;
      }

      // Spawn projectile from drone position
      ecs::SpawnEntityEvent droneSpawnEvent(projectileType, droneTransform.x + droneOffsetX,
                                            droneTransform.y + droneOffsetY,
                                            player); // Owner is still the player for scoring
      world.emitEvent(droneSpawnEvent);
    }
  }
};

} // namespace server

#endif // SERVER_SHOOTING_SYSTEM_HPP_
