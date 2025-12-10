/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem
*/

#include "NetworkReceiveSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"

NetworkReceiveSystem::NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkReceiveSystem::~NetworkReceiveSystem() {}

void NetworkReceiveSystem::update(ecs::World &world, float deltaTime)
{
  NetworkPacket packet;
  while (m_networkManager->poll(packet)) {
    std::string message =
      m_networkManager->getPacketHandler()->deserialize(packet.getData(), packet.getBytesTransferred());

    auto type = nlohmann::json::parse(message)["type"].get<std::string>(); // A definir un protocol
    if (type == "player_input") {
      handlePlayerInput(world, message, packet.getSenderEndpointId());
    }
  }
}

ecs::ComponentSignature NetworkReceiveSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Networked>());
  return sig;
}

void NetworkReceiveSystem::handlePlayerInput(ecs::World &world, std::string message, std::uint32_t clientId) {}