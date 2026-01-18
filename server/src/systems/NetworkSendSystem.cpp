/**
 * @file NetworkSendSystem.cpp
 * @brief Server-side network send system implementation.
 */

#include "systems/NetworkSendSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "INetworkManager.hpp"
#include "LobbyManager.hpp"
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <span>
#include <string>
#include <vector>

NetworkSendSystem::NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkSendSystem::~NetworkSendSystem() {}

void NetworkSendSystem::setLobbyManager(LobbyManager *lobbyManager)
{
  m_lobbyManager = lobbyManager;
}

std::vector<std::uint32_t> NetworkSendSystem::getActiveGameClients() const
{
  std::vector<std::uint32_t> activeClients;

  if (m_lobbyManager == nullptr) {
    // Fallback: send to all connected clients
    const auto clients = m_networkManager->getClients();
    for (const auto &[clientId, _endpoint] : clients) {
      activeClients.push_back(clientId);
    }
    return activeClients;
  }

  // Only get clients from lobbies where game has started
  for (const auto &[code, lobby] : m_lobbyManager->getLobbies()) {
    if (lobby && lobby->isGameStarted()) {
      for (const auto &clientId : lobby->getClients()) {
        activeClients.push_back(clientId);
      }
    }
  }

  return activeClients;
}

void NetworkSendSystem::update(UNUSED ecs::World &world, float deltaTime)
{
  static float logAccumulator = 0.0f;
  logAccumulator += deltaTime;

  m_timeSinceLastSend += deltaTime;
  if (m_timeSinceLastSend >= SEND_INTERVAL) {

    // Clean up cached last-network-id lists for lobbies that no longer exist
    // or that are not currently running to avoid sending stale 'destroyed' lists.
    if (m_lobbyManager != nullptr) {
      std::vector<std::string> toErase;
      for (const auto &pair : m_lobbyLastNetworkIds) {
        const std::string &code = pair.first;
        auto lobbyIt = m_lobbyManager->getLobbies().find(code);
        if (lobbyIt == m_lobbyManager->getLobbies().end() || lobbyIt->second == nullptr ||
            !lobbyIt->second->isGameStarted()) {
          toErase.push_back(code);
        }
      }
      for (const auto &code : toErase) {
        m_lobbyLastNetworkIds.erase(code);
      }
    }

    // If no lobby manager, skip (can't send lobby-specific state)
    if (m_lobbyManager == nullptr) {
      m_timeSinceLastSend = 0.0f;
      return;
    }

    // Send snapshots per-lobby: each lobby gets only its own entities
    for (const auto &[code, lobby] : m_lobbyManager->getLobbies()) {
      if (!lobby || !lobby->isGameStarted()) {
        continue;
      }

      auto lobbyWorld = lobby->getWorld();
      if (!lobbyWorld) {
        continue;
      }

      // Get entities from THIS lobby's world
      std::vector<ecs::Entity> entities;
      lobbyWorld->getEntitiesWithSignature(getSignature(), entities);

      nlohmann::json snapshot;
      snapshot["type"] = "snapshot";
      snapshot["entities"] = nlohmann::json::array();

      // Collect current network IDs for this lobby
      std::vector<uint32_t> currentNetworkIds;

      for (const auto &entity : entities) {
        if (!lobbyWorld->hasComponent<ecs::Transform>(entity) || !lobbyWorld->isAlive(entity)) {
          continue;
        }

        const auto &networked = lobbyWorld->getComponent<ecs::Networked>(entity);
        const auto &transform = lobbyWorld->getComponent<ecs::Transform>(entity);

        currentNetworkIds.push_back(networked.networkId);

        nlohmann::json entityJson;
        entityJson["id"] = networked.networkId;
        entityJson["transform"] = {
          {"x", transform.x}, {"y", transform.y}, {"rotation", transform.rotation}, {"scale", transform.scale}};

        if (lobbyWorld->hasComponent<ecs::Collider>(entity)) {
          const auto &col = lobbyWorld->getComponent<ecs::Collider>(entity);
          entityJson["collider"] = {{"w", col.width}, {"h", col.height}};
        }

        // SERVER-DRIVEN SPRITE REPLICATION
        if (lobbyWorld->hasComponent<ecs::Sprite>(entity)) {
          const auto &sprite = lobbyWorld->getComponent<ecs::Sprite>(entity);
          entityJson["sprite"] = sprite.toJson();
        }

        // Replicate health for HUD display
        if (lobbyWorld->hasComponent<ecs::Health>(entity)) {
          const auto &health = lobbyWorld->getComponent<ecs::Health>(entity);
          entityJson["health"] = {{"hp", health.hp}, {"maxHp", health.maxHp}};
        }

        // Replicate score for HUD display
        if (lobbyWorld->hasComponent<ecs::Score>(entity)) {
          const auto &score = lobbyWorld->getComponent<ecs::Score>(entity);
          entityJson["score"] = {{"points", score.points}};
        }

        // Include owner client id when present so client can identify its player reliably
        if (lobbyWorld->hasComponent<ecs::PlayerId>(entity)) {
          const auto &pid = lobbyWorld->getComponent<ecs::PlayerId>(entity);
          entityJson["owner_client"] = pid.clientId;
        }

        snapshot["entities"].push_back(entityJson);
      }

      // Track entities that should be removed on client side (per-lobby)
      auto &lastIds = m_lobbyLastNetworkIds[code];
      std::vector<uint32_t> destroyedIds;
      for (const auto &lastId : lastIds) {
        if (std::find(currentNetworkIds.begin(), currentNetworkIds.end(), lastId) == currentNetworkIds.end()) {
          destroyedIds.push_back(lastId);
        }
      }

      if (!destroyedIds.empty()) {
        snapshot["destroyed"] = destroyedIds;
      }

      // Update last network IDs for this lobby
      lastIds = currentNetworkIds;

      // Send ONLY to clients in THIS lobby
      const auto &lobbyClients = lobby->getClients();
      if (lobbyClients.empty()) {
        continue;
      }

      const std::string jsonStr = snapshot.dump();
      const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

      for (const auto &clientId : lobbyClients) {
        m_networkManager->send(
          std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()),
          clientId);
      }

      if (logAccumulator >= 1.0f) {
        std::cout << "[Lobby:" << code << "] Snapshot: entities=" << snapshot["entities"].size()
                  << " clients=" << lobbyClients.size() << std::endl;
      }
    }

    if (logAccumulator >= 1.0f) {
      logAccumulator = 0.0f;
    }

    m_timeSinceLastSend = 0.0f;
  }
}

ecs::ComponentSignature NetworkSendSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Networked>());
  return sig;
}
