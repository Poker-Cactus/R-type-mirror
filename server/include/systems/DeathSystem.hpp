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
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../Lobby.hpp"
#include "../WorldLobbyRegistry.hpp"
#include "ecs/ComponentSignature.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
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
      if (!world.isAlive(entity)) {
        continue;
      }
      const auto &health = world.getComponent<ecs::Health>(entity);
      if (health.hp <= 0) {
        toDie.push_back(entity);
      }
    }

    // Destroy dead entities
    for (auto entity : toDie) {
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
    // When an entity dies, award score to the killer
    if (world.isAlive(event.killer)) {
      // Award 100 points to the killer
      ecs::ScoreEvent scoreEvent(event.killer, 100);
      world.emitEvent(scoreEvent);
      std::cout << "[DeathSystem] Entity " << event.entity << " killed by " << event.killer << " - awarding 100 points"
                << std::endl;
    } else {
      std::cout << "[DeathSystem] Entity " << event.entity << " died but killer " << event.killer << " is not alive"
                << std::endl;
    }
    // Notify owning client (if any) that their player died so client can return to menu.
    // Try to find the lobby owning this world and send a direct message.
    Lobby *lobby = getLobbyForWorld(&world);
    if (lobby != nullptr) {
      if (world.isAlive(event.entity) && world.hasComponent<ecs::PlayerId>(event.entity)) {
        const auto &pid = world.getComponent<ecs::PlayerId>(event.entity);
        nlohmann::json msg;
        msg["type"] = "player_dead";
        msg["reason"] = "killed";
        // Include final HP (should be 0) and score if available
        if (world.hasComponent<ecs::Health>(event.entity)) {
          const auto &health = world.getComponent<ecs::Health>(event.entity);
          msg["hp"] = health.hp;
          msg["maxHp"] = health.maxHp;
        } else {
          msg["hp"] = 0;
          msg["maxHp"] = 0;
        }
        if (world.hasComponent<ecs::Score>(event.entity)) {
          const auto &score = world.getComponent<ecs::Score>(event.entity);
          msg["score"] = score.points;
        } else {
          msg["score"] = 0;
        }

        lobby->sendJsonToClient(pid.clientId, msg);
      }
    }

    // Actual destruction happens in update()
  }
};

} // namespace server

#endif // SERVER_DEATH_SYSTEM_HPP_
