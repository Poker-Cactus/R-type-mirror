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
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
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
bool g_acceptSnapshots = false;
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

    // Ignore simple protocol-level keepalive/debug messages that are not JSON
    if (message == "PING" || message == "PONG") {
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

      // Server told us that this player is dead
      if (type == "player_dead" || type == "player_died_spectate") {
        std::cout << "[Client] Received " << type << " from server" << std::endl;
        // Only stop accepting snapshots for full game over (player_dead)
        if (type == "player_dead") {
          g_acceptSnapshots = false;
        }
        // For player_died_spectate, we continue receiving snapshots as spectator
        if (m_playerDeadCallback) {
          m_playerDeadCallback(json);
        }
        continue;
      }

      // Server told us we've been kicked
      if (type == "player_kicked") {
        std::cout << "[Client] Received player_kicked from server" << std::endl;
        // Stop accepting snapshots immediately
        g_acceptSnapshots = false;
        if (m_playerDeadCallback) {
          m_playerDeadCallback(json);
        }
        continue;
      }

      // Lobby left acknowledgement from server
      if (type == "lobby_left") {
        std::cout << "[Client] Received lobby_left from server" << std::endl;
        // Stop accepting snapshots immediately
        g_acceptSnapshots = false;
        if (m_lobbyStateCallback) {
          // Notify any lobby UI about the updated state (0 players)
          m_lobbyStateCallback(json.value("code", ""), 0);
        }
        // Call optional lobby-left callback
        if (m_lobbyLeftCallback) {
          m_lobbyLeftCallback();
        }
        continue;
      }

      // Game entity messages
      if (type == "entity_created") {
        handleEntityCreated(world, json);
      } else if (type == "entity_update") {
        handleEntityUpdate(world, json);
      } else if (type == "snapshot") {
        // Only process snapshots when allowed (we may have left the lobby)
        if (g_acceptSnapshots) {
          handleSnapshot(world, json);
        }
      } else if (type == "game_started") {
        handleGameStarted();
      }
      // Lobby messages
      else if (type == "lobby_joined") {
        std::string code = json.value("code", "");
        std::cout << "[Client] Joined lobby: " << code << std::endl;

        // Clear existing entities and network id mapping when joining a lobby
        // to avoid leftover entities from previous lobbies causing visual/HP glitches.
        try {
          // Destroy all existing entities in the world
          ecs::ComponentSignature emptySig; // default empty signature matches all
          std::vector<ecs::Entity> allEntities;
          world.getEntitiesWithSignature(emptySig, allEntities);
          for (auto e : allEntities) {
            if (world.isAlive(e)) {
              world.destroyEntity(e);
            }
          }
          // Clear client-side mapping of network ids to entities
          g_networkIdToEntity.clear();
        } catch (const std::exception &e) {
          std::cerr << "[Client] Error clearing world on lobby join: " << e.what() << std::endl;
        }

        if (m_lobbyJoinedCallback) {
          m_lobbyJoinedCallback(code);
        }
      } else if (type == "lobby_state") {
        std::string code = json.value("code", "");
        int playerCount = json.value("player_count", 0);
        std::cout << "[Client] Lobby " << code << " has " << playerCount << " players" << std::endl;
        if (m_lobbyStateCallback) {
          m_lobbyStateCallback(code, playerCount);
        }
      } else if (type == "error") {
        std::string errorMsg = json.value("message", "Unknown error");
        std::cerr << "[Client] Server error: " << errorMsg << std::endl;
        if (m_errorCallback) {
          m_errorCallback(errorMsg);
        }
      } else if (type == "chat_broadcast") {
        // Handle incoming chat message from server
        std::string sender = json.value("sender", "Unknown");
        std::string content = json.value("content", "");
        std::uint32_t senderId = json.value("senderId", 0);
        std::cout << "[Client] Chat from " << sender << ": " << content << std::endl;
        if (m_chatMessageCallback) {
          m_chatMessageCallback(sender, content, senderId);
        }
      } else if (type == "level_complete") {
        // Handle level complete event from server
        std::string currentLevel = json.value("current_level", "");
        std::string nextLevel = json.value("next_level", "");
        std::cout << "[Client] ✓ Level complete: " << currentLevel << " → " << nextLevel << std::endl;
        if (m_levelCompleteCallback) {
          m_levelCompleteCallback(currentLevel, nextLevel);
        }
      }

    } catch (const std::exception &e) {
      std::cerr << "[Client] Error parsing message: " << e.what() << std::endl;
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
        // New sprite: add it completely
        world.addComponent(entity, sprite);
      } else {
        // Existing sprite: update only non-animated fields to preserve client animation state
        auto &existingSprite = world.getComponent<ecs::Sprite>(entity);

        // Preserve animation state (currentFrame and animationTimer are client-managed)
        uint32_t preservedCurrentFrame = existingSprite.currentFrame;
        float preservedAnimationTimer = existingSprite.animationTimer;

        // Update sprite from server
        existingSprite = sprite;

        // Restore client animation state
        existingSprite.currentFrame = preservedCurrentFrame;
        existingSprite.animationTimer = preservedAnimationTimer;
      }
    }

    // Owner/client id: set PlayerId component so client can identify its player entity
    if (entityJson.contains("owner_client") && entityJson["owner_client"].is_number_unsigned()) {
      const std::uint32_t ownerClient = entityJson["owner_client"].get<std::uint32_t>();
      if (!world.hasComponent<ecs::PlayerId>(entity)) {
        ecs::PlayerId pid{.clientId = ownerClient};
        world.addComponent(entity, pid);
      } else {
        auto &pid = world.getComponent<ecs::PlayerId>(entity);
        pid.clientId = ownerClient;
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

  // ---- Concise receive/display logging (throttled and change-detected) ----
  // Determine our client id
  auto *sendSys = world.getSystem<NetworkSendSystem>();
  if (sendSys != nullptr) {
    const std::uint32_t myClientId = sendSys->getClientId();

    // Find the entity that belongs to us (prefer PlayerId.owner_client, fallback to networkId)
    ecs::Entity myEntity = 0;
    // First pass: PlayerId
    std::vector<ecs::Entity> candidateEntities;
    {
      ecs::ComponentSignature sig;
      sig.set(ecs::getComponentId<ecs::Networked>());
      // We only need candidates which have Networked (that's what snapshots replicate)
      world.getEntitiesWithSignature(sig, candidateEntities);
    }

    for (auto e : candidateEntities) {
      if (world.hasComponent<ecs::PlayerId>(e)) {
        const auto &pid = world.getComponent<ecs::PlayerId>(e);
        if (pid.clientId == myClientId) {
          myEntity = e;
          break;
        }
      }
    }

    if (myEntity == 0) {
      // Fallback: match by Networked.networkId == myClientId
      for (auto e : candidateEntities) {
        if (!world.hasComponent<ecs::Networked>(e))
          continue;
        const auto &net = world.getComponent<ecs::Networked>(e);
        if (static_cast<std::uint32_t>(net.networkId) == myClientId) {
          myEntity = e;
          break;
        }
      }
    }

    int displayedHp = -1;
    int displayedMaxHp = -1;
    int displayedScore = -1;

    if (myEntity != 0 && world.isAlive(myEntity)) {
      if (world.hasComponent<ecs::Health>(myEntity)) {
        const auto &h = world.getComponent<ecs::Health>(myEntity);
        displayedHp = h.hp;
        displayedMaxHp = h.maxHp;
      }
      if (world.hasComponent<ecs::Score>(myEntity)) {
        const auto &s = world.getComponent<ecs::Score>(myEntity);
        displayedScore = s.points;
      }
    }

    // Static previous values to detect changes and throttle logs
    static int prevHp = -9999;
    static int prevScore = -9999;
    static int tickCounter = 0;
    ++tickCounter;

    bool changed = (displayedHp != prevHp) || (displayedScore != prevScore);
    // Log if changed or every 120 snapshots (~2s at 60Hz snapshots)
    if (changed || (tickCounter % 120) == 0) {
      std::cout << "[Client][RECV] snapshot entities=" << json["entities"].size() << " clientId=" << myClientId
                << " entity=" << myEntity << " hp=" << displayedHp << "/" << displayedMaxHp
                << " score=" << displayedScore << std::endl;
      // Also echo what the HUD will display (concise)
      std::cout << "[Client][DISPLAY] HP=" << (displayedHp >= 0 ? std::to_string(displayedHp) : "n/a")
                << " Score=" << (displayedScore >= 0 ? std::to_string(displayedScore) : "n/a") << std::endl;
      prevHp = displayedHp;
      prevScore = displayedScore;
    }
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

void ClientNetworkReceiveSystem::setGameStartedCallback(std::function<void()> callback)
{
  m_gameStartedCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::setLobbyJoinedCallback(std::function<void(const std::string &)> callback)
{
  m_lobbyJoinedCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::setLobbyStateCallback(std::function<void(const std::string &, int)> callback)
{
  m_lobbyStateCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::setErrorCallback(std::function<void(const std::string &)> callback)
{
  m_errorCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::handleGameStarted()
{
  std::cout << "[Client] Received game_started message from server" << std::endl;

  // Allow snapshots once the game starts
  g_acceptSnapshots = true;

  if (m_gameStartedCallback) {
    m_gameStartedCallback();
  }
}

void ClientNetworkReceiveSystem::setAcceptSnapshots(bool accept)
{
  g_acceptSnapshots = accept;
}

void ClientNetworkReceiveSystem::setLobbyLeftCallback(std::function<void()> callback)
{
  m_lobbyLeftCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::setPlayerDeadCallback(std::function<void(const nlohmann::json &)> callback)
{
  m_playerDeadCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::setChatMessageCallback(
  std::function<void(const std::string &, const std::string &, std::uint32_t)> callback)
{
  m_chatMessageCallback = std::move(callback);
}

void ClientNetworkReceiveSystem::setLevelCompleteCallback(
  std::function<void(const std::string &, const std::string &)> callback)
{
  m_levelCompleteCallback = std::move(callback);
}

ecs::ComponentSignature ClientNetworkReceiveSystem::getSignature() const
{
  return ecs::ComponentSignature(); // No specific filter needed
}
