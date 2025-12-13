/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem
*/

#include "systems/NetworkReceiveSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <unordered_set>

namespace
{
std::unordered_set<std::uint32_t> g_loggedFirstInputFromClient;
}

NetworkReceiveSystem::NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkReceiveSystem::~NetworkReceiveSystem() {}

void NetworkReceiveSystem::update(ecs::World &world, float deltaTime)
{
  NetworkPacket packet;
  while (m_networkManager->poll(packet)) {
    std::string message = m_networkManager->getPacketHandler()
                            ->deserialize(packet.getData(), packet.getBytesTransferred())
                            .value_or("");

    if (message.empty()) {
      continue;
    }

    try {
      auto type = nlohmann::json::parse(message)["type"].get<std::string>(); // A definir un protocol
      if (type == "player_input") {
        handlePlayerInput(world, message, packet.getSenderEndpointId());
      }
    } catch (const std::exception &) {
      continue;
    }
  }
}

ecs::ComponentSignature NetworkReceiveSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Networked>());
  return sig;
}

void NetworkReceiveSystem::handlePlayerInput(ecs::World &world, std::string message, std::uint32_t clientId)
{
  try {
    auto json = nlohmann::json::parse(message);

    if (!json.contains("input") || !json["input"].is_object()) {
      return;
    }

    const auto &inputJson = json["input"];

    ecs::Entity targetEntity = 0;
    if (json.contains("entity_id") && json["entity_id"].is_number_unsigned()) {
      targetEntity = json["entity_id"].get<ecs::Entity>();
    }

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (const auto &entity : entities) {
      auto &networked = world.getComponent<ecs::Networked>(entity);

      if (targetEntity != 0) {
        if (networked.networkId != targetEntity) {
          continue;
        }
      } else {
        if (networked.networkId != clientId) {
          continue;
        }
      }

      if (!world.hasComponent<ecs::Input>(entity)) {
        return;
      }

      auto &input = world.getComponent<ecs::Input>(entity);
      input.up = inputJson.value("up", false);
      input.down = inputJson.value("down", false);
      input.left = inputJson.value("left", false);
      input.right = inputJson.value("right", false);
      input.shoot = inputJson.value("shoot", false);

      if (!g_loggedFirstInputFromClient.contains(clientId)) {
        std::cout << "[Server] First input received from client " << clientId << " (entity=" << networked.networkId
                  << ")" << std::endl;
        g_loggedFirstInputFromClient.insert(clientId);
      }
      return;
    }
  } catch (const std::exception &) {
    return;
  }
}