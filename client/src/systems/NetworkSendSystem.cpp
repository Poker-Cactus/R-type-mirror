/**
 * @file NetworkSendSystem.cpp
 * @brief Client-side input transmission system implementation.
 */

#include "../../include/systems/NetworkSendSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include <iostream>
#include <optional>

namespace
{
constexpr float SEND_INTERVAL = 0.016f;
}

NetworkSendSystem::NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = networkManager;
}

NetworkSendSystem::~NetworkSendSystem() {}

void NetworkSendSystem::update(ecs::World &world, float deltaTime)
{
  static float timeSinceLastSend = 0.0f;
  timeSinceLastSend += deltaTime;
  if (timeSinceLastSend < SEND_INTERVAL) {
    return;
  }
  timeSinceLastSend = 0.0f;

  // Get all entities with Input component
  std::vector<ecs::Entity> entities;
  world.getEntitiesWithSignature(getSignature(), entities);

  // Send inputs for each player-controlled entity
  for (const auto &entity : entities) {
    if (!world.hasComponent<ecs::Input>(entity)) {
      continue;
    }

    const auto &input = world.getComponent<ecs::Input>(entity);

    // Send only input state to the server at fixed rate.
    sendInputToServer(entity, input);
  }
}

ecs::ComponentSignature NetworkSendSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Input>());
  return sig;
}

void NetworkSendSystem::sendInputToServer(UNUSED ecs::Entity entity, const ecs::Input &input)
{
  struct InputState {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
    bool chargedShoot;
    bool detach;
  };

  static std::optional<InputState> last;
  static float logAccumulator = 0.0f;
  logAccumulator += SEND_INTERVAL;

  const InputState now{input.up, input.down, input.left, input.right, input.shoot, input.chargedShoot, input.detach};
  const bool changed = !last.has_value() || last->up != now.up || last->down != now.down || last->left != now.left ||
    last->right != now.right || last->shoot != now.shoot || last->chargedShoot != now.chargedShoot ||
    last->detach != now.detach;

  // Create JSON message with player input
  nlohmann::json message;
  message["type"] = "player_input";
  // Send the server-assigned client id as the authoritative player identifier.
  // The server applies input to the entity owned by this clientId.
  message["entity_id"] = m_clientId;
  message["input"]["up"] = input.up;
  message["input"]["down"] = input.down;
  message["input"]["left"] = input.left;
  message["input"]["right"] = input.right;
  message["input"]["shoot"] = input.shoot;
  message["input"]["chargedShoot"] = input.chargedShoot;
  message["input"]["detach"] = input.detach;

  // Serialize JSON to string
  std::string jsonStr = message.dump();

  // Use Cap'n Proto handler to serialize the message
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  // Send to server (endpoint ID 0 for client -> server communication)
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

  // Low-noise logging: print on change, and also periodically (2Hz) to confirm activity.
  if (changed || logAccumulator >= 0.5f) {
    // Minimal send log — avoid flooding. Detailed inspection is done on receive/display.
    std::cout << "[Client][SEND] input updated (client_id=" << m_clientId << ")" << std::endl;
    logAccumulator = 0.0f;
  }

  last = now;
}

void NetworkSendSystem::sendSetDifficulty(Difficulty diff)
{
  nlohmann::json message;
  message["type"] = "set_difficulty";
  std::string diffStr;
  switch (diff) {
  case Difficulty::EASY:
    diffStr = "easy";
    break;
  case Difficulty::MEDIUM:
    diffStr = "medium";
    break;
  case Difficulty::EXPERT:
    diffStr = "expert";
    break;
  }
  message["difficulty"] = diffStr;

  const std::string serialized = message.dump();
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

  std::cout << "[Client][SEND] set difficulty to " << diffStr << std::endl;
}
