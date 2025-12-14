/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem
*/

#include "systems/NetworkSendSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "INetworkManager.hpp"
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

void NetworkSendSystem::update(ecs::World &world, float deltaTime)
{
  static float logAccumulator = 0.0F;
  logAccumulator += deltaTime;

  m_timeSinceLastSend += deltaTime;
  if (m_timeSinceLastSend >= SEND_INTERVAL) {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    nlohmann::json snapshot;
    snapshot["type"] = "snapshot";
    snapshot["entities"] = nlohmann::json::array();

    // Collect current network IDs
    std::vector<uint32_t> currentNetworkIds;

    for (const auto &entity : entities) {
      if (!world.hasComponent<ecs::Transform>(entity) || !world.isAlive(entity)) {
        continue;
      }

      const auto &networked = world.getComponent<ecs::Networked>(entity);
      const auto &transform = world.getComponent<ecs::Transform>(entity);

      currentNetworkIds.push_back(networked.networkId);

      nlohmann::json entityJson;
      entityJson["id"] = networked.networkId;
      entityJson["transform"] = {
        {"x", transform.x}, {"y", transform.y}, {"rotation", transform.rotation}, {"scale", transform.scale}};

      if (world.hasComponent<ecs::Collider>(entity)) {
        const auto &col = world.getComponent<ecs::Collider>(entity);
        entityJson["collider"] = {{"w", col.width}, {"h", col.height}};
      }

      // SERVER-DRIVEN SPRITE REPLICATION
      // Visual identity is replicated as data - client never infers it
      if (world.hasComponent<ecs::Sprite>(entity)) {
        const auto &sprite = world.getComponent<ecs::Sprite>(entity);
        entityJson["sprite"] = sprite.toJson();
      }

      // Replicate health for HUD display
      if (world.hasComponent<ecs::Health>(entity)) {
        const auto &health = world.getComponent<ecs::Health>(entity);
        entityJson["health"] = {{"hp", health.hp}, {"maxHp", health.maxHp}};
      }

      // Replicate score for HUD display
      if (world.hasComponent<ecs::Score>(entity)) {
        const auto &score = world.getComponent<ecs::Score>(entity);
        entityJson["score"] = {{"points", score.points}};
      }

      snapshot["entities"].push_back(entityJson);
    }

    // Track entities that should be removed on client side
    // Find network IDs that existed last frame but not this frame
    std::vector<uint32_t> destroyedIds;
    for (const auto &lastId : m_lastNetworkIds) {
      if (std::find(currentNetworkIds.begin(), currentNetworkIds.end(), lastId) == currentNetworkIds.end()) {
        destroyedIds.push_back(lastId);
      }
    }

    if (!destroyedIds.empty()) {
      snapshot["destroyed"] = destroyedIds;
    }

    // Update last network IDs for next frame
    m_lastNetworkIds = currentNetworkIds;

    const std::string jsonStr = snapshot.dump();
    const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

    const auto clients = m_networkManager->getClients();
    for (const auto &[clientId, _endpoint] : clients) {
      m_networkManager->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()),
        clientId);
    }

    if (logAccumulator >= 1.0F) {
      std::cout << "[Server] Broadcasting snapshot: entities=" << entities.size()
                << " clients=" << clients.size() << std::endl;
      logAccumulator = 0.0F;
    }

    m_timeSinceLastSend = 0.0F;
  }
}

ecs::ComponentSignature NetworkSendSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Networked>());
  return sig;
}