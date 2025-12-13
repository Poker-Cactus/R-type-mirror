/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem
*/

#include "systems/NetworkSendSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "INetworkManager.hpp"
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include <cstddef>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <span>
#include <string>
#include <vector>
#include <iostream>

NetworkSendSystem::NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkSendSystem::~NetworkSendSystem() {}

void NetworkSendSystem::update(ecs::World &world, float deltaTime)
{
  static float logAccumulator = 0.0f;
  logAccumulator += deltaTime;

  m_timeSinceLastSend += deltaTime;
  if (m_timeSinceLastSend >= SEND_INTERVAL) {
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    nlohmann::json snapshot;
    snapshot["type"] = "snapshot";
    snapshot["entities"] = nlohmann::json::array();

    for (const auto &entity : entities) {
      if (!world.hasComponent<ecs::Transform>(entity)) {
        continue;
      }

      const auto &networked = world.getComponent<ecs::Networked>(entity);
      const auto &transform = world.getComponent<ecs::Transform>(entity);

      snapshot["entities"].push_back({
        {"id", networked.networkId},
        {"transform", {{"x", transform.x}, {"y", transform.y}, {"rotation", transform.rotation}, {"scale", transform.scale}}}
      });
    }

    const std::string jsonStr = snapshot.dump();
    const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

    const auto clients = m_networkManager->getClients();
    for (const auto &[clientId, _endpoint] : clients) {
      m_networkManager->send(
        std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), clientId);
    }

    if (logAccumulator >= 1.0f) {
      std::cout << "[Server] Snapshot broadcast: entities=" << snapshot["entities"].size() << " clients="
                << clients.size() << std::endl;
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