/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem - Client-side input transmission
*/

#include "../../include/systems/NetworkSendSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include <iostream>

NetworkSendSystem::NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkSendSystem::~NetworkSendSystem() {}

void NetworkSendSystem::update(ecs::World &world, float deltaTime)
{
  (void)deltaTime;

  // Get all entities with Input component
  std::vector<ecs::Entity> entities;
  world.getEntitiesWithSignature(getSignature(), entities);

  // Send inputs for each player-controlled entity
  for (const auto &entity : entities) {
    if (!world.hasComponent<ecs::Input>(entity)) {
      continue;
    }

    const auto &input = world.getComponent<ecs::Input>(entity);

    // Send input to server every frame to keep server synchronized
    sendInputToServer(entity, input);
  }
}

ecs::ComponentSignature NetworkSendSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Input>());
  return sig;
}

void NetworkSendSystem::sendInputToServer(ecs::Entity entity, const ecs::Input &input)
{
  // Create JSON message with player input
  nlohmann::json message;
  message["type"] = "player_input";
  message["entity_id"] = entity;
  message["input"]["up"] = input.up;
  message["input"]["down"] = input.down;
  message["input"]["left"] = input.left;
  message["input"]["right"] = input.right;
  message["input"]["shoot"] = input.shoot;

  // Serialize JSON to string
  std::string jsonStr = message.dump();

  // Use Cap'n Proto handler to serialize the message
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  // Send to server (endpoint ID 0 for client -> server communication)
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

  (void)entity;
  (void)input;
}