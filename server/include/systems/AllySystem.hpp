/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AllySystem.hpp - ECS System that controls ally entities using AI logic
*/

#ifndef SERVER_ALLY_SYSTEM_HPP_
#define SERVER_ALLY_SYSTEM_HPP_

#include "../../../engineCore/include/ecs/Entity.hpp"
#include "../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../engineCore/include/ecs/World.hpp"
#include "../../../engineCore/include/ecs/components/Ally.hpp"
#include "../../../engineCore/include/ecs/components/Charging.hpp"
#include "../ai/AllyAI.hpp"
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
        // Get ally strength from component
        auto &allyComponent = world.getComponent<ecs::Ally>(allyEntity);
        // Create new AI controller
        m_allyControllers[allyEntity] = std::make_unique<ai::AllyAI>(allyComponent.strength);
      }

      // Update the AI
      m_allyControllers[allyEntity]->update(world, allyEntity, deltaTime);
    }

    // Update charging for allies
    updateAllyCharging(world, deltaTime);

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

  /**
   * @brief Update charging state for ally entities
   */
  void updateAllyCharging(ecs::World &world, float deltaTime)
  {
    // Get all ally entities that have charging components
    ecs::ComponentSignature chargingSig;
    chargingSig.set(ecs::getComponentId<ecs::Ally>());
    chargingSig.set(ecs::getComponentId<ecs::Charging>());
    std::vector<ecs::Entity> chargingAllies;
    world.getEntitiesWithSignature(chargingSig, chargingAllies);

    for (auto allyEntity : chargingAllies) {
      if (!world.isAlive(allyEntity)) {
        continue;
      }

      auto &charging = world.getComponent<ecs::Charging>(allyEntity);

      // Update charge time
      if (charging.isCharging) {
        charging.chargeTime += deltaTime;

        // Auto-fire when charge is complete
        if (charging.chargeTime >= charging.maxChargeTime) {
          constexpr float CHARGED_OFFSET_X = 105.0F;
          constexpr float CHARGED_OFFSET_Y = 25.0F;

          auto &allyTransform = world.getComponent<ecs::Transform>(allyEntity);
          float spawnX = allyTransform.x + CHARGED_OFFSET_X;
          float spawnY = allyTransform.y + CHARGED_OFFSET_Y;

          ecs::SpawnEntityEvent spawnEvent(ecs::SpawnEntityEvent::EntityType::CHARGED_PROJECTILE, spawnX, spawnY,
                                           allyEntity);
          world.emitEvent(spawnEvent);

          // Destroy loading shot animation
          if (charging.loadingShotEntity != 0 && world.isAlive(charging.loadingShotEntity)) {
            world.destroyEntity(charging.loadingShotEntity);
          }

          // Reset charging state
          charging.isCharging = false;
          charging.chargeTime = 0.0F;
          charging.loadingShotEntity = 0;
        }
      }
    }
  }

private:
  // Map of ally entities to their AI controllers
  std::map<ecs::Entity, std::unique_ptr<ai::AllyAI>> m_allyControllers;
};

} // namespace server

#endif // SERVER_ALLY_SYSTEM_HPP_
