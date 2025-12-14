/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem (Client)
*/

#include "../../include/systems/NetworkReceiveSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../include/systems/NetworkSendSystem.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace
{
bool g_loggedFirstSnapshot = false;
std::unordered_map<std::uint32_t, ecs::Entity> g_networkIdToEntity;
float g_debugLogAcc = 0.0F;
} // namespace

ClientNetworkReceiveSystem::ClientNetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager)
    : m_networkManager(networkManager)
{
}

ClientNetworkReceiveSystem::~ClientNetworkReceiveSystem() = default;

void ClientNetworkReceiveSystem::update(ecs::World &world, float deltaTime)
{
  g_debugLogAcc += deltaTime;

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

      if (type == "assign_id") {
        if (json.contains("client_id") && json["client_id"].is_number_unsigned()) {
          const std::uint32_t clientId = json["client_id"].get<std::uint32_t>();
          // Inform the send system about our server-assigned id.
          if (auto *sendSys = world.getSystem<NetworkSendSystem>()) {
            sendSys->setClientId(clientId);
          }
          std::cout << "[Client] Assigned client_id=" << clientId << std::endl;
        }
        continue;
      }

      if (type == "entity_created") {
        handleEntityCreated(world, json);
      } else if (type == "entity_update") {
        handleEntityUpdate(world, json);
      } else if (type == "snapshot") {
        handleSnapshot(world, json);
      }

    } catch (const std::exception &e) {
      std::cerr << "Client: Erreur parsing: " << e.what() << std::endl;
    }
  }
}

void ClientNetworkReceiveSystem::handleSnapshot(ecs::World &world, const nlohmann::json &json)
{
  if (!json.contains("entities") || !json["entities"].is_array()) {
    return;
  }

  if (!g_loggedFirstSnapshot) {
    std::cout << "[Client] Snapshot received (entities=" << json["entities"].size() << ")" << std::endl;
    g_loggedFirstSnapshot = true;
  }

  for (const auto &entityJson : json["entities"]) {
    if (!entityJson.contains("id") || !entityJson.contains("transform")) {
      continue;
    }
    if (!entityJson["transform"].is_object()) {
      continue;
    }

    const std::uint32_t networkId = entityJson["id"].get<std::uint32_t>();
    const auto &transformJson = entityJson["transform"];

    const float x = transformJson.value("x", 0.0F);
    const float y = transformJson.value("y", 0.0F);
    const float rotation = transformJson.value("rotation", 0.0F);
    const float scale = transformJson.value("scale", 1.0F);

    ecs::Entity entity = 0;
    auto it = g_networkIdToEntity.find(networkId);
    if (it != g_networkIdToEntity.end()) {
      entity = it->second;
    } else {
      entity = world.createEntity();
      g_networkIdToEntity.emplace(networkId, entity);
      ecs::Networked net;
      net.networkId = static_cast<ecs::Entity>(networkId);
      world.addComponent(entity, net);
    }

    if (!world.hasComponent<ecs::Transform>(entity)) {
      ecs::Transform transform;
      transform.x = x;
      transform.y = y;
      transform.rotation = rotation;
      transform.scale = scale;
      world.addComponent(entity, transform);
    }

    auto &transform = world.getComponent<ecs::Transform>(entity);
    transform.x = x;
    transform.y = y;
    transform.rotation = rotation;
    transform.scale = scale;

    if (entityJson.contains("collider") && entityJson["collider"].is_object()) {
      const auto &colJ = entityJson["collider"];
      const float w = colJ.value("w", 0.0F);
      const float h = colJ.value("h", 0.0F);
      if (w > 0.0F && h > 0.0F) {
        if (!world.hasComponent<ecs::Collider>(entity)) {
          world.addComponent(entity, ecs::Collider{w, h});
        } else {
          auto &col = world.getComponent<ecs::Collider>(entity);
          col.width = w;
          col.height = h;
        }
      }
    }

    // CLIENT RECEIVES SERVER-DRIVEN SPRITE DATA
    // Visual identity is never inferred - only applied from server
    if (entityJson.contains("sprite") && entityJson["sprite"].is_object()) {
      const auto &spriteJson = entityJson["sprite"];
      ecs::Sprite sprite = ecs::Sprite::fromJson(spriteJson);
      
      if (!world.hasComponent<ecs::Sprite>(entity)) {
        world.addComponent(entity, sprite);
      } else {
        world.getComponent<ecs::Sprite>(entity) = sprite;
      }
    }

    // Receive health data for HUD display
    if (entityJson.contains("health") && entityJson["health"].is_object()) {
      const auto &healthJson = entityJson["health"];
      const int hp = healthJson.value("hp", 0);
      const int maxHp = healthJson.value("maxHp", 100);
      
      if (!world.hasComponent<ecs::Health>(entity)) {
        ecs::Health health;
        health.hp = hp;
        health.maxHp = maxHp;
        world.addComponent(entity, health);
      } else {
        auto &health = world.getComponent<ecs::Health>(entity);
        health.hp = hp;
        health.maxHp = maxHp;
      }
    }

    // Receive score data for HUD display
    if (entityJson.contains("score") && entityJson["score"].is_object()) {
      const auto &scoreJson = entityJson["score"];
      const int points = scoreJson.value("points", 0);
      
      if (!world.hasComponent<ecs::Score>(entity)) {
        ecs::Score score;
        score.points = points;
        world.addComponent(entity, score);
      } else {
        auto &score = world.getComponent<ecs::Score>(entity);
        score.points = points;
      }
    }

  }

  // Handle destroyed entities
  if (json.contains("destroyed") && json["destroyed"].is_array()) {
    for (const auto &destroyedId : json["destroyed"]) {
      if (!destroyedId.is_number_unsigned()) {
        continue;
      }
      
      const std::uint32_t networkId = destroyedId.get<std::uint32_t>();
      auto it = g_networkIdToEntity.find(networkId);
      if (it != g_networkIdToEntity.end()) {
        ecs::Entity entity = it->second;
        if (world.isAlive(entity)) {
          world.destroyEntity(entity);
        }
        g_networkIdToEntity.erase(it);
      }
    }
  }

  if (g_debugLogAcc >= 1.0F) {
    g_debugLogAcc = 0.0F;
  }
}

void ClientNetworkReceiveSystem::handleEntityCreated(ecs::World &world, const nlohmann::json &json)
{
  const std::uint32_t networkId = json["entity_id"].get<std::uint32_t>();
  float x = json["position"]["x"].get<float>();
  float y = json["position"]["y"].get<float>();

  ecs::Entity entity = 0;
  auto it = g_networkIdToEntity.find(networkId);
  if (it != g_networkIdToEntity.end()) {
    entity = it->second;
  } else {
    entity = world.createEntity();
    g_networkIdToEntity.emplace(networkId, entity);
    ecs::Networked net;
    net.networkId = static_cast<ecs::Entity>(networkId);
    world.addComponent(entity, net);
  }

  if (!world.hasComponent<ecs::Transform>(entity)) {
    ecs::Transform transform;
    transform.x = x;
    transform.y = y;
    transform.rotation = 0.0F;
    transform.scale = 1.0F;
    world.addComponent(entity, transform);
  } else {
    auto &transform = world.getComponent<ecs::Transform>(entity);
    transform.x = x;
    transform.y = y;
  }
}

void ClientNetworkReceiveSystem::handleEntityUpdate(ecs::World &world, const nlohmann::json &json)
{
  const std::uint32_t networkId = json["entity_id"].get<std::uint32_t>();

  auto it = g_networkIdToEntity.find(networkId);
  if (it == g_networkIdToEntity.end()) {
    return;
  }

  const ecs::Entity entity = it->second;

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

  (void)lastProcessed;
}

ecs::ComponentSignature ClientNetworkReceiveSystem::getSignature() const
{
  return ecs::ComponentSignature(); // Pas de filtre spécifique
}