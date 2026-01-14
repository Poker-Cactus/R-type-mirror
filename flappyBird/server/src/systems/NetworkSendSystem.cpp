/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem - FlappyBird Server Implementation
*/

#include "systems/NetworkSendSystem.hpp"
#include "../../../../engineCore/include/ecs/World.hpp"

NetworkSendSystem::NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager)
    : m_networkManager(std::move(networkManager))
{
}

NetworkSendSystem::~NetworkSendSystem() {}

void NetworkSendSystem::update([[maybe_unused]] ecs::World &world, [[maybe_unused]] float deltaTime)
{
  // For now, this system is just a placeholder
  // In the future, it will send game state updates to clients
}

ecs::ComponentSignature NetworkSendSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  // No specific components required for now
  return sig;
}
