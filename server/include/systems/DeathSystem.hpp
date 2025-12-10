/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** DeathSystem.hpp - Handles entity death and cleanup
*/

#ifndef SERVER_DEATH_SYSTEM_HPP_
#define SERVER_DEATH_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Health.hpp"
#include "../../../engineCore/include/ecs/events/EventListenerHandle.hpp"
#include "../../../engineCore/include/ecs/events/GameEvents.hpp"
#include "ecs/ComponentSignature.hpp"
#include <vector>

namespace server
{

/**
 * @brief System that destroys entities when they die
 */
class DeathSystem : public ecs::ISystem
{
public:
  DeathSystem() = default;

  void update(ecs::World &world, float deltaTime) override
  {
    (void)deltaTime;

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    std::vector<ecs::Entity> toDie;

    for (auto entity : entities) {
      const auto &health = world.getComponent<ecs::Health>(entity);
      if (health.hp <= 0) {
        toDie.push_back(entity);
      }
    }

    // Destroy dead entities
    for (auto entity : toDie) {
      // Emit score event for destroyed entity
      ecs::ScoreEvent scoreEvent(0, 100);
      world.emitEvent(scoreEvent);

      world.destroyEntity(entity);
    }
  }

  /**
   * @brief Subscribe to death events for special handling
   */
  void initialize(ecs::World &world)
  {
    m_deathHandle =
      world.subscribeEvent<ecs::DeathEvent>([&world](const ecs::DeathEvent &event) { handleDeath(world, event); });
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Health>());
    return sig;
  }

private:
  ecs::EventListenerHandle m_deathHandle;

  static void handleDeath(ecs::World &world, const ecs::DeathEvent &event)
  {
    (void)world;
    (void)event;
    // Could spawn explosion effect, play sound, etc.
    // For now, the actual destruction happens in update()
  }
};

} // namespace server

#endif // SERVER_DEATH_SYSTEM_HPP_
