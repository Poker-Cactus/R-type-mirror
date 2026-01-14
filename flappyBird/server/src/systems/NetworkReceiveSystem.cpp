/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem - FlappyBird Server Implementation
*/

#include "systems/NetworkReceiveSystem.hpp"
#include "../../../../engineCore/include/ecs/World.hpp"
#include "../../../../engineCore/include/ecs/components/Input.hpp"
#include "Game.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <span>

NetworkReceiveSystem::NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager, Game *game)
    : m_networkManager(std::move(networkManager)), m_game(game)
{
}

NetworkReceiveSystem::~NetworkReceiveSystem() {}

void NetworkReceiveSystem::update(ecs::World &world, [[maybe_unused]] float deltaTime)
{
  NetworkPacket packet;
  while (m_networkManager->poll(packet)) {
    const std::uint32_t clientId = packet.getSenderEndpointId();

    const std::string message =
      m_networkManager->getPacketHandler()->deserialize(packet.getData(), packet.getBytesTransferred()).value_or("");

    if (message.empty()) {
      std::cerr << "[FlappyBird Server] Empty or malformed message from client " << clientId << '\n';
      continue;
    }

    handleMessage(world, message, clientId);
  }
}

ecs::ComponentSignature NetworkReceiveSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Input>());
  return sig;
}

void NetworkReceiveSystem::handleMessage(ecs::World &world, const std::string &message, std::uint32_t clientId)
{
  // Handle simple protocol messages
  if (message == "PING") {
    auto pong = m_networkManager->getPacketHandler()->serialize("PONG");
    m_networkManager->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(pong.data()), pong.size()),
                           clientId);
    return;
  }

  if (message == "PONG") {
    return;
  }

  try {
    const auto json = nlohmann::json::parse(message);

    if (!json.contains("type")) {
      std::cerr << "[FlappyBird Server] Message has no 'type' field from client " << clientId << '\n';
      return;
    }

    const std::string type = json["type"].get<std::string>();

    if (type == "connect") {
      handleConnect(clientId);
    } else if (type == "disconnect") {
      handleDisconnect(clientId);
    } else if (type == "player_input") {
      handlePlayerInput(world, message, clientId);
    } else {
      std::cout << "[FlappyBird Server] Received message type: " << type << " from client " << clientId << '\n';
    }

  } catch (const std::exception &e) {
    std::cerr << "[FlappyBird Server] Exception handling message from client " << clientId << ": " << e.what() << '\n';
  }
}

void NetworkReceiveSystem::handlePlayerInput(ecs::World &world, const std::string &message, std::uint32_t clientId)
{
  try {
    const auto json = nlohmann::json::parse(message);

    // For FlappyBird, we mainly care about jump input
    if (json.contains("jump") && json["jump"].get<bool>()) {
      std::cout << "[FlappyBird Server] Client " << clientId << " jumped!" << '\n';
      // TODO: Handle jump logic in a separate system
    }

  } catch (const std::exception &e) {
    std::cerr << "[FlappyBird Server] Error parsing player input: " << e.what() << '\n';
  }
}

void NetworkReceiveSystem::handleConnect(std::uint32_t clientId)
{
  if (m_game) {
    m_game->addClient(clientId);

    // Send welcome message
    nlohmann::json welcome;
    welcome["type"] = "welcome";
    welcome["client_id"] = clientId;
    welcome["message"] = "Connected to FlappyBird server";

    sendJsonMessage(clientId, welcome);
  }
}

void NetworkReceiveSystem::handleDisconnect(std::uint32_t clientId)
{
  if (m_game) {
    m_game->removeClient(clientId);
  }
}

void NetworkReceiveSystem::sendJsonMessage(std::uint32_t clientId, const nlohmann::json &message)
{
  const std::string jsonStr = message.dump();
  const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), clientId);
}
