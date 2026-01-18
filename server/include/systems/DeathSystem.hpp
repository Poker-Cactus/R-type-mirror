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
#include "../../../engineCore/include/ecs/components/Immortal.hpp"
#include "../../../engineCore/include/ecs/components/Input.hpp"
#include "../../../engineCore/include/ecs/components/Lifetime.hpp"
#include "../../../engineCore/include/ecs/components/Networked.hpp"
#include "../../../engineCore/include/ecs/components/Shield.hpp"
#include "../../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../../engineCore/include/ecs/components/Transform.hpp"
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
      // Don't show death animation for players
      bool isPlayer = world.hasComponent<ecs::Input>(entity);

      if (!isPlayer && world.hasComponent<ecs::Transform>(entity) && !world.hasComponent<ecs::Shield>(entity)) {
        // Spawn death animation for enemies
        spawnDeathAnimation(world, entity);
      }

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

  static void spawnDeathAnimation(ecs::World &world, ecs::Entity deadEntity)
  {
    // Get position of dead entity
    const auto &transform = world.getComponent<ecs::Transform>(deadEntity);

    // Create death animation entity
    ecs::Entity deathAnim = world.createEntity();

    // Position
    ecs::Transform animTransform;
    animTransform.x = transform.x;
    animTransform.y = transform.y;
    animTransform.rotation = 0.0F;
    animTransform.scale = 1.0F;
    world.addComponent(deathAnim, animTransform);

    // Sprite configuration for death animation
    // Image: 586x94, 6 frames → each frame is ~98px wide
    ecs::Sprite sprite;
    sprite.spriteId = ecs::SpriteId::DEATH_ANIM;
    sprite.width = 98; // 586 / 6
    sprite.height = 94;
    sprite.animated = true;
    sprite.frameCount = 6;
    sprite.currentFrame = 0;
    sprite.startFrame = 0;
    sprite.endFrame = 5;
    sprite.frameTime = 0.07F; // 100ms per frame = 600ms total animation
    sprite.loop = false; // One-shot animation
    sprite.animationTimer = 0.0F;
    sprite.reverseAnimation = false;
    world.addComponent(deathAnim, sprite);

    // Network replication
    ecs::Networked net;
    net.networkId = deathAnim;
    world.addComponent(deathAnim, net);

    // Add lifetime: animation duration = 6 frames * 0.05s = 0.3s
    // Set slightly higher to ensure last frame is displayed
    ecs::Lifetime lifetime;
    lifetime.remaining = 0.35F; // 350ms = animation duration + minimal buffer
    world.addComponent(deathAnim, lifetime);
  }

  static void handleDeath(ecs::World &world, const ecs::DeathEvent &event)
  {
    // If a shield dies, remove immortality from its parent
    if (world.isAlive(event.entity) && world.hasComponent<ecs::Shield>(event.entity)) {
      const auto &shield = world.getComponent<ecs::Shield>(event.entity);
      if (world.isAlive(shield.parent) && world.hasComponent<ecs::Immortal>(shield.parent)) {
        auto &immortal = world.getComponent<ecs::Immortal>(shield.parent);
        immortal.isImmortal = false;
        std::cout << "[DeathSystem] Shield destroyed, removing immortality from parent " << shield.parent << std::endl;
      }
    }

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
    Lobby *lobby = getLobbyForWorld(&world);
    if (lobby != nullptr) {
      if (world.isAlive(event.entity) && world.hasComponent<ecs::PlayerId>(event.entity)) {
        const auto &pid = world.getComponent<ecs::PlayerId>(event.entity);

        std::cout << "[DeathSystem] Player " << pid.clientId << " died. Counting remaining alive players..."
                  << std::endl;

        // Count remaining alive players (non-spectators), excluding the current dying player
        int alivePlayerCount = 0;
        int totalClients = 0;
        int spectatorCount = 0;

        for (const auto &clientId : lobby->getClients()) {
          totalClients++;

          // Skip spectators
          if (lobby->isSpectator(clientId)) {
            spectatorCount++;
            std::cout << "[DeathSystem]   Client " << clientId << ": SPECTATOR (skipping)" << std::endl;
            continue;
          }

          // Skip the player who is dying
          if (clientId == pid.clientId) {
            std::cout << "[DeathSystem]   Client " << clientId << ": DYING PLAYER (skipping)" << std::endl;
            continue;
          }

          ecs::Entity playerEntity = lobby->getPlayerEntity(clientId);
          std::cout << "[DeathSystem]   Client " << clientId << ": entity=" << playerEntity;

          // Check if entity is alive (don't check != 0 because entity 0 is valid)
          if (world.isAlive(playerEntity) && world.hasComponent<ecs::Health>(playerEntity)) {
            const auto &health = world.getComponent<ecs::Health>(playerEntity);
            std::cout << " hp=" << health.hp << "/" << health.maxHp;
            if (health.hp > 0) {
              alivePlayerCount++;
              std::cout << " -> ALIVE" << std::endl;
            } else {
              std::cout << " -> DEAD" << std::endl;
            }
          } else {
            std::cout << " -> NO VALID ENTITY (not alive or no health)" << std::endl;
          }
        }

        std::cout << "[DeathSystem] Summary: totalClients=" << totalClients << " spectators=" << spectatorCount
                  << " alive=" << alivePlayerCount << std::endl;

        nlohmann::json msg;

        // If there are still alive players, convert dead player to spectator
        if (alivePlayerCount > 0) {
          std::cout << "[DeathSystem] -> Sending player_died_spectate" << std::endl;
          msg["type"] = "player_died_spectate";
          msg["reason"] = "killed";
          msg["alive_players"] = alivePlayerCount;

          // Convert player to spectator in the lobby
          lobby->convertToSpectator(pid.clientId);

        } else {
          std::cout << "[DeathSystem] -> Sending player_dead (game over)" << std::endl;
          // Last player died - game over for everyone
          msg["type"] = "player_dead";
          msg["reason"] = "game_over";
        }
        if (world.hasComponent<ecs::Health>(event.entity)) {
          const auto &health = world.getComponent<ecs::Health>(event.entity);
          msg["hp"] = health.hp;
        }
        if (world.hasComponent<ecs::Score>(event.entity)) {
          const auto &score = world.getComponent<ecs::Score>(event.entity);
          msg["score"] = score.points;
        }
        lobby->sendJsonToClient(pid.clientId, msg);
      }
    }

    // Actual destruction happens in update()
  }
};

} // namespace server

#endif // SERVER_DEATH_SYSTEM_HPP_
