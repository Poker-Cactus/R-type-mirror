/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllySystem.hpp - ECS System that controls ally entities using AI logic
*/

#ifndef SERVER_ALLY_SYSTEM_HPP_
#define SERVER_ALLY_SYSTEM_HPP_

#include "../ai/AllyAI.hpp"
#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "ecs/ComponentSignature.hpp"
#include <map>

namespace server
{

/**
 * @brief ECS System that controls ally entities
 *
 * This system orchestrates the AI behavior of ally-controlled entities.
 * It manages the lifecycle of AI controllers and updates them each frame.
 */
class AllySystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    // Check if solo mode (only one player)
    std::vector<ecs::Entity> players;
    ecs::ComponentSignature playerSig;
    playerSig.set(ecs::getComponentId<ecs::PlayerId>());
    world.getEntitiesWithSignature(playerSig, players);
    bool isSoloMode = (players.size() == 1);

    if (!isSoloMode) {
      return; // Only process allies in solo mode
    }

    // Get all ally entities
    std::vector<ecs::Entity> allyEntities;
    world.getEntitiesWithSignature(getSignature(), allyEntities);

    // Update or create AI controllers for each ally
    for (auto allyEntity : allyEntities) {
      if (!world.isAlive(allyEntity))
        continue;

      // Get or create AI controller for this ally
      auto it = m_allyControllers.find(allyEntity);
      if (it == m_allyControllers.end()) {
        // Create new AI controller
        m_allyControllers[allyEntity] = std::make_unique<ai::AllyAI>();
      }

      // Update the AI
      m_allyControllers[allyEntity]->update(world, allyEntity, deltaTime);
    }

    // Clean up dead allies
    for (auto it = m_allyControllers.begin(); it != m_allyControllers.end();) {
      if (!world.isAlive(it->first)) {
        it = m_allyControllers.erase(it);
      } else {
        ++it;
      }
    }
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Ally>());
    sig.set(ecs::getComponentId<ecs::Transform>());
    sig.set(ecs::getComponentId<ecs::Velocity>());
    return sig;
  }

private:
  // Map of ally entities to their AI controllers
  std::map<ecs::Entity, std::unique_ptr<ai::AllyAI>> m_allyControllers;
};

} // namespace server

#endif // SERVER_ALLY_SYSTEM_HPP_
