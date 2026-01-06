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
      const auto &transform = world.getComponent<ecs::Transform>(entity);

      const bool wasShooting = m_prevShootState.contains(entity) ? m_prevShootState[entity] : false;
      const bool justPressed = input.shoot && !wasShooting;

      if (justPressed && canShoot(entity)) {
        // Emit shoot event
        ecs::ShootEvent shootEvent(entity, 1.0F, 0.0F);
        world.emitEvent(shootEvent);

        m_lastShootTime[entity] = m_currentTime;
      }

      m_prevShootState[entity] = input.shoot;
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
  std::unordered_map<ecs::Entity, bool> m_prevShootState;
  float m_currentTime = 0.0F;
  const float SHOOT_COOLDOWN = 0.05F; // 5 shots per second

  bool canShoot(ecs::Entity entity)
  {
    auto iter = m_lastShootTime.find(entity);
    if (iter == m_lastShootTime.end()) {
      return true;
    }
    return (m_currentTime - iter->second) >= SHOOT_COOLDOWN;
  }

  static void spawnProjectile(ecs::World &world, const ecs::ShootEvent &event)
  {
    if (!world.isAlive(event.shooter)) {
      return;
    }

    const auto &transform = world.getComponent<ecs::Transform>(event.shooter);

    const float offsetX = 110.0F;
    const float offsetY = 25.0F;

    // Emit spawn event for projectile
    ecs::SpawnEntityEvent spawnEvent(ecs::SpawnEntityEvent::EntityType::PROJECTILE, transform.x + offsetX, transform.y + offsetY,
                                     event.shooter);
    world.emitEvent(spawnEvent);
  }
};

} // namespace server

#endif // SERVER_SHOOTING_SYSTEM_HPP_
