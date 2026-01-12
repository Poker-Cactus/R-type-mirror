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
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
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

      // Tir chargé - utiliser le même système d'événements
      const bool wasChargedShooting =
        m_prevChargedShootState.contains(entity) ? m_prevChargedShootState[entity] : false;
      const bool justChargedPressed = input.chargedShoot && !wasChargedShooting;

      if (justChargedPressed && canChargedShoot(entity)) { // Utiliser canChargedShoot au lieu de canShoot
        const auto &transform = world.getComponent<ecs::Transform>(entity);

        // Émettre un événement de spawn pour un projectile chargé
        ecs::SpawnEntityEvent spawnEvent(ecs::SpawnEntityEvent::EntityType::CHARGED_PROJECTILE, // Nouveau type
                                         transform.x + 105.0F, transform.y + 25.0F, entity);
        world.emitEvent(spawnEvent);

        m_lastChargedShootTime[entity] = m_currentTime;
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
  const float CHARGED_SHOOT_COOLDOWN = 2.0F; // 3 charged shot per second

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

  static void spawnChargedProjectile(ecs::World &world, ecs::Entity shooter)
  {
    if (!world.isAlive(shooter)) {
      return;
    }

    const auto &transform = world.getComponent<ecs::Transform>(shooter);

    const float offsetX = 105.0F;
    const float offsetY = 25.0F;

    ecs::Entity projectile = world.createEntity();

    ecs::Transform projTransform;
    projTransform.x = transform.x + offsetX;
    projTransform.y = transform.y + offsetY;
    projTransform.scale = 2.0F;
    world.addComponent(projectile, projTransform);

    ecs::Velocity velocity;
    velocity.dx = 800.0F;
    velocity.dy = 0.0F;
    world.addComponent(projectile, velocity);

    // Collider plus grand
    world.addComponent(projectile, ecs::Collider{16.0F, 16.0F});

    // Sprite du projectile chargé
    // ecs::Sprite sprite;
    // sprite.spriteId = ecs::SpriteId::CHARGED_PROJECTILE; // Utiliser le nouveau sprite
    // sprite.width = 100;
    // sprite.height = 50;
    // sprite.animated = false;
    // world.addComponent(projectile, sprite);

    // ecs::Networked net;
    // net.networkId = projectile;
    // world.addComponent(projectile, net);

    // ecs::Owner ownerComp;
    // ownerComp.ownerId = shooter;
    // world.addComponent(projectile, ownerComp);
  }
};

} // namespace server

#endif // SERVER_SHOOTING_SYSTEM_HPP_
