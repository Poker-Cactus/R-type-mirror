/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem (Client)
*/

#include "../../include/systems/NetworkReceiveSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

ClientNetworkReceiveSystem::ClientNetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager)
    : m_networkManager(networkManager)
{
}

ClientNetworkReceiveSystem::~ClientNetworkReceiveSystem() = default;

void ClientNetworkReceiveSystem::update(ecs::World &world, float deltaTime)
{
  (void)deltaTime;

  NetworkPacket packet;
  while (m_networkManager->poll(packet)) {
    std::string message =
      m_networkManager->getPacketHandler()->deserialize(packet.getData(), packet.getBytesTransferred()).value_or("");

    if (message.empty()) {
      continue;
    }

    try {
      auto json = nlohmann::json::parse(message);
      std::string type = json["type"].get<std::string>();

      if (type == "entity_created") {
        handleEntityCreated(world, json);
      } else if (type == "entity_update") {
        handleEntityUpdate(world, json);
      }

    } catch (const std::exception &e) {
      std::cerr << "Client: Erreur parsing: " << e.what() << std::endl;
    }
  }
}

void ClientNetworkReceiveSystem::handleEntityCreated(ecs::World &world, const nlohmann::json &json)
{
  ecs::Entity entity = json["entity_id"].get<ecs::Entity>();
  float x = json["position"]["x"].get<float>();
  float y = json["position"]["y"].get<float>();

  // Créer l'entité côté client avec le même ID
  ecs::Transform transform;
  transform.x = x;
  transform.y = y;
  world.addComponent(entity, transform);

  std::cout << "✓ Client: Entité " << entity << " créée à (" << x << ", " << y << ")" << std::endl;
}

void ClientNetworkReceiveSystem::handleEntityUpdate(ecs::World &world, const nlohmann::json &json)
{
  ecs::Entity entity = json["entity_id"].get<ecs::Entity>();

  if (!world.hasComponent<ecs::Transform>(entity)) {
    return;
  }

  float x = json["position"]["x"].get<float>();
  float y = json["position"]["y"].get<float>();
  float rotation = json["rotation"].get<float>();
  std::uint32_t lastProcessed = json["last_processed_input"].get<std::uint32_t>();

  auto &transform = world.getComponent<ecs::Transform>(entity);
  transform.x = x;
  transform.y = y;
  transform.rotation = rotation;

  std::cout << "📥 Client: Entité " << entity << " mise à jour -> (" << x << ", " << y << ") [input #" << lastProcessed
            << "]" << std::endl;
}

ecs::ComponentSignature ClientNetworkReceiveSystem::getSignature() const
{
  return ecs::ComponentSignature(); // Pas de filtre spécifique
}