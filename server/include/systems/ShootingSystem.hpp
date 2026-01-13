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

      if (justPressed && canShoot(entity)) {
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
  std::unordered_map<ecs::Entity, bool> m_prevChargedShootState;

  float m_currentTime = 0.0F;
  const float SHOOT_COOLDOWN = 0.05F; // 5 shots per second
  const float CHARGED_SHOOT_COOLDOWN = 2.0F; // Cooldown between charged shots

  bool canShoot(ecs::Entity entity)
  {
    auto iter = m_lastShootTime.find(entity);
    if (iter == m_lastShootTime.end()) {
      return true;
    }
    return (m_currentTime - iter->second) >= SHOOT_COOLDOWN;
  }

  bool canChargedShoot(ecs::Entity entity)
  {
    auto iter = m_lastChargedShootTime.find(entity);
    if (iter == m_lastChargedShootTime.end()) {
      return true;
    }
    return (m_currentTime - iter->second) >= CHARGED_SHOOT_COOLDOWN;
  }

  static void spawnProjectile(ecs::World &world, const ecs::ShootEvent &event)
  {
    if (!world.isAlive(event.shooter)) {
      return;
    }

    const auto &transform = world.getComponent<ecs::Transform>(event.shooter);

    const float offsetX = 105.0F;
    const float offsetY = 25.0F;

    // Emit spawn event for projectile
    ecs::SpawnEntityEvent spawnEvent(ecs::SpawnEntityEvent::EntityType::PROJECTILE, transform.x + offsetX,
                                     transform.y + offsetY, event.shooter);
    world.emitEvent(spawnEvent);
  }
};

} // namespace server

#endif // SERVER_SHOOTING_SYSTEM_HPP_
