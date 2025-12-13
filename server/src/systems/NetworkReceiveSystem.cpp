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
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../engineCore/include/ecs/components/Viewport.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <unordered_set>

namespace
{
std::unordered_set<std::uint32_t> g_loggedFirstInputFromClient;
float g_inputLogAccumulator = 0.0f;
}

NetworkReceiveSystem::NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkReceiveSystem::~NetworkReceiveSystem() {}

void NetworkReceiveSystem::update(ecs::World &world, float deltaTime)
{
  g_inputLogAccumulator += deltaTime;

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
      } else if (type == "viewport") {
        // Client tells us its current window size so we can clamp server-authoritatively.
        try {
          auto json = nlohmann::json::parse(message);
          if (!json.contains("width") || !json.contains("height")) {
            continue;
          }
          if (!json["width"].is_number_unsigned() || !json["height"].is_number_unsigned()) {
            continue;
          }

          const std::uint32_t width = json["width"].get<std::uint32_t>();
          const std::uint32_t height = json["height"].get<std::uint32_t>();

          std::vector<ecs::Entity> entities;
          world.getEntitiesWithSignature(getSignature(), entities);
          for (const auto &entity : entities) {
            const auto &owner = world.getComponent<ecs::PlayerId>(entity);
            if (owner.clientId != packet.getSenderEndpointId()) {
              continue;
            }

            if (!world.hasComponent<ecs::Viewport>(entity)) {
              ecs::Viewport vp;
              vp.width = width;
              vp.height = height;
              world.addComponent(entity, vp);
            } else {
              auto &vp = world.getComponent<ecs::Viewport>(entity);
              vp.width = width;
              vp.height = height;
            }
            break;
          }
        } catch (...) {
          continue;
        }
      }
    } catch (const std::exception &) {
      continue;
    }
  }
}

ecs::ComponentSignature NetworkReceiveSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::PlayerId>());
  sig.set(ecs::getComponentId<ecs::Input>());
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

    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    bool matched = false;

    for (const auto &entity : entities) {
      const auto &owner = world.getComponent<ecs::PlayerId>(entity);
      if (owner.clientId != clientId) {
        continue;
      }

      auto &input = world.getComponent<ecs::Input>(entity);
      input.up = inputJson.value("up", false);
      input.down = inputJson.value("down", false);
      input.left = inputJson.value("left", false);
      input.right = inputJson.value("right", false);
      input.shoot = inputJson.value("shoot", false);

      matched = true;

      if (g_inputLogAccumulator >= 0.5f) {
        std::cout << "[Server] Applied input client=" << clientId
                  << " up=" << input.up << " down=" << input.down << " left=" << input.left
                  << " right=" << input.right << " shoot=" << input.shoot << std::endl;
        g_inputLogAccumulator = 0.0f;
      }

      if (!g_loggedFirstInputFromClient.contains(clientId)) {
        std::cout << "[Server] First input received from client " << clientId << std::endl;
        g_loggedFirstInputFromClient.insert(clientId);
      }
      return;
    }

    if (!matched && g_inputLogAccumulator >= 0.5f) {
      std::cout << "[Server] Input received but no owned player for client=" << clientId
                << " (ownedCandidates=" << entities.size() << ")" << std::endl;
      g_inputLogAccumulator = 0.0f;
    }

    if (!matched && g_inputLogAccumulator >= 0.5f) {
      std::cout << "[Server] Input received but no owned player for client=" << clientId
                << " (ownedCandidates=" << entities.size() << ")" << std::endl;
      g_inputLogAccumulator = 0.0f;
    }
  } catch (const std::exception &) {
    return;
  }
}