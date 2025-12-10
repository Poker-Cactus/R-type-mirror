/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem
*/

#include "NetworkSendSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"

NetworkSendSystem::NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkSendSystem::~NetworkSendSystem() {}

void NetworkSendSystem::update(ecs::World &world, float deltaTime)
{
  m_timeSinceLastSend += deltaTime;
  if (m_timeSinceLastSend >= SEND_INTERVAL) {
    std::vector<Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (const auto &entity : entities) {
      auto &networked = world.getComponent<ecs::Networked>(entity);

      nlohmann::json jsonData = networked.toJson();

      std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> clients = m_networkManager->getClients();

      for (const auto &[clientId, endpoint] : clients) {
        m_networkManager->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(jsonData.dump().data()),
          jsonData.dump().size()), clientId);
      }
    }
  }
  m_timeSinceLastSend = 0.0f;
}

ecs::ComponentSignature NetworkSendSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Networked>());
  return sig;
}