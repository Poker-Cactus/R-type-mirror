/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem
*/

#include "systems/NetworkReceiveSystem.hpp"
#include "../../engineCore/include/ecs/World.hpp"
// LobbyResponseData is defined in the header
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../engineCore/include/ecs/components/Viewport.hpp"
#include "Game.hpp"
#include "Lobby.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <span>

NetworkReceiveSystem::NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = std::move(networkManager);
}

NetworkReceiveSystem::~NetworkReceiveSystem() {}

void NetworkReceiveSystem::setGame(Game *game)
{
  m_game = game;
}

void NetworkReceiveSystem::update(ecs::World &world, [[maybe_unused]] float deltaTime)
{
  NetworkPacket packet;
  while (m_networkManager->poll(packet)) {
    const std::uint32_t clientId = packet.getSenderEndpointId();

    const std::string message =
      m_networkManager->getPacketHandler()->deserialize(packet.getData(), packet.getBytesTransferred()).value_or("");

    if (message.empty()) {
      std::cerr << "[Server] Empty or malformed message received from client " << clientId << '\n';
      continue;
    }

    handleMessage(world, message, clientId);
  }
}
ecs::ComponentSignature NetworkReceiveSystem::getSignature() const
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::PlayerId>());
  sig.set(ecs::getComponentId<ecs::Input>());
  return sig;
}

void NetworkReceiveSystem::handleMessage(ecs::World &world, const std::string &message, std::uint32_t clientId)
{
  // Ignore simple protocol-level keepalive/debug messages that are not JSON
  if (message == "PING") {
    // Respond with PONG
    auto pong = m_networkManager->getPacketHandler()->serialize("PONG");
    m_networkManager->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(pong.data()), pong.size()), clientId);
    return;
  }
  if (message == "PONG") {
    return;
  }

  try {
    const auto json = nlohmann::json::parse(message);
    if (!json.contains("type")) {
      std::cerr << "[Server] Message from client " << clientId << " has no 'type' field" << '\n';
      return;
    }

    const std::string type = json["type"].get<std::string>();

    if (type == "player_input") {
      handlePlayerInput(world, message, clientId);
    } else if (type == "request_lobby") {
      handleRequestLobby(json, clientId);
    } else if (type == "start_game") {
      handleStartGame(world, clientId);
    } else if (type == "leave_lobby") {
      handleLeaveLobby(world, clientId);
    } else if (type == "viewport") {
      handleViewport(world, json, clientId);
    } else if (type == "set_difficulty") {
      handleSetDifficulty(json, clientId);
    } else {
      std::cerr << "[Server] Unknown message type from client " << clientId << ": " << type << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "[Server] Exception handling message from client " << clientId << ": " << e.what() << '\n';
    return;
  }
}
void NetworkReceiveSystem::handleViewport([[maybe_unused]] ecs::World &world, const nlohmann::json &json,
                                          std::uint32_t clientId)
{
  if (!json.contains("width") || !json.contains("height")) {
    return;
  }

  if (!json["width"].is_number_unsigned() || !json["height"].is_number_unsigned()) {
    return;
  }

  const std::uint32_t width = json["width"].get<std::uint32_t>();
  const std::uint32_t height = json["height"].get<std::uint32_t>();

  // Get the lobby's world instead of the main world
  if (m_game == nullptr) {
    return;
  }

  auto *lobby = m_game->getLobbyManager().getClientLobby(clientId);
  if (lobby == nullptr || !lobby->isGameStarted()) {
    return;
  }

  auto lobbyWorld = lobby->getWorld();
  if (!lobbyWorld) {
    return;
  }

  std::vector<ecs::Entity> entities;
  lobbyWorld->getEntitiesWithSignature(getSignature(), entities);

  for (const auto &entity : entities) {
    const auto &owner = lobbyWorld->getComponent<ecs::PlayerId>(entity);
    if (owner.clientId != clientId) {
      continue;
    }

    if (!lobbyWorld->hasComponent<ecs::Viewport>(entity)) {
      ecs::Viewport viewport{.width = width, .height = height};
      lobbyWorld->addComponent(entity, viewport);
    } else {
      auto &viewport = lobbyWorld->getComponent<ecs::Viewport>(entity);
      viewport.width = width;
      viewport.height = height;
    }
    break;
  }
}

void NetworkReceiveSystem::handlePlayerInput([[maybe_unused]] ecs::World &world, std::string message,
                                             std::uint32_t clientId)
{
  try {
    auto json = nlohmann::json::parse(message);

    if (!json.contains("input") || !json["input"].is_object()) {
      return;
    }

    // Get the lobby's world instead of the main world
    if (m_game == nullptr) {
      return;
    }

    auto *lobby = m_game->getLobbyManager().getClientLobby(clientId);
    if (lobby == nullptr || !lobby->isGameStarted()) {
      return;
    }

    auto lobbyWorld = lobby->getWorld();
    if (!lobbyWorld) {
      return;
    }

    const auto &inputJson = json["input"];
    std::vector<ecs::Entity> entities;
    lobbyWorld->getEntitiesWithSignature(getSignature(), entities);

    for (const auto &entity : entities) {
      const auto &owner = lobbyWorld->getComponent<ecs::PlayerId>(entity);
      if (owner.clientId != clientId) {
        continue;
      }

      auto &input = lobbyWorld->getComponent<ecs::Input>(entity);
      input.up = inputJson.value("up", false);
      input.down = inputJson.value("down", false);
      input.left = inputJson.value("left", false);
      input.right = inputJson.value("right", false);
      input.shoot = inputJson.value("shoot", false);
      return;
    }
  } catch (const std::exception &) {
    return;
  }
}

void NetworkReceiveSystem::handleStartGame([[maybe_unused]] ecs::World &world, std::uint32_t clientId)
{
  if (m_game == nullptr) {
    return;
  }

  auto *lobby = m_game->getLobbyManager().getClientLobby(clientId);
  if (lobby == nullptr) {
    sendErrorResponse(clientId, "Not in any lobby");
    return;
  }

  // Check if this lobby's game is already started
  if (lobby->isGameStarted()) {
    sendErrorResponse(clientId, "Game already in progress");
    return;
  }

  std::cout << "[Server] Player " << clientId << " started game in lobby: " << lobby->getCode() << '\n';

  // Start game for this specific lobby - this initializes systems and spawns players
  // The Lobby::startGame() method handles all initialization internally
  lobby->startGame();

  // Send game_started to all players in the lobby using proper serialization
  nlohmann::json message;
  message["type"] = "game_started";

  std::vector<std::uint32_t> lobbyClients(lobby->getClients().begin(), lobby->getClients().end());
  sendJsonMessageToAll(lobbyClients, message);
}

void NetworkReceiveSystem::handleRequestLobby(const nlohmann::json &json, std::uint32_t clientId)
{
  std::cout << "[Server] === RECEIVED LOBBY REQUEST ===" << '\n';
  std::cout << "[Server] Client ID: " << clientId << '\n';
  std::cout << "[Server] Full JSON message: " << json.dump() << '\n';

  if (m_game == nullptr) {
    std::cerr << "[Server] Error: m_game is nullptr!" << '\n';
    sendErrorResponse(clientId, "Server error");
    return;
  }

  auto &lobbyManager = m_game->getLobbyManager();

  // Get action from request (default to "create" for backwards compatibility)
  const std::string action = json.value("action", "create");
  const std::string requestedCode = json.value("lobby_code", "");

  std::string lobbyCode;
  Lobby *targetLobby = nullptr;

  if (action == "join" && !requestedCode.empty()) {
    // Client wants to join a specific lobby
    targetLobby = lobbyManager.getLobby(requestedCode);
    if (targetLobby == nullptr) {
      std::cerr << "[Server] Client " << clientId << " tried to join non-existent lobby: " << requestedCode << '\n';
      sendErrorResponse(clientId, "Lobby not found: " + requestedCode);
      return;
    }
    if (targetLobby->isGameStarted()) {
      std::cerr << "[Server] Client " << clientId << " tried to join lobby in progress: " << requestedCode << '\n';
      sendErrorResponse(clientId, "Lobby game already in progress");
      return;
    }
    lobbyCode = requestedCode;
    std::cout << "[Server] Client " << clientId << " joining existing lobby: " << lobbyCode << '\n';
  } else {
    // Client wants to create a new lobby (or find any available)
    // Create a new lobby with simple numeric code
    static int lobbyCounter = 0;
    lobbyCode = std::to_string(++lobbyCounter);

    // Parse difficulty
    GameConfig::Difficulty difficulty = GameConfig::Difficulty::MEDIUM;
    if (json.contains("difficulty")) {
      int diffInt = json["difficulty"];
      std::cout << "[Server] >>> DIFFICULTY RECEIVED: " << diffInt;
      if (diffInt == 0) std::cout << " (EASY)";
      else if (diffInt == 1) std::cout << " (MEDIUM)";
      else if (diffInt == 2) std::cout << " (EXPERT)";
      else std::cout << " (INVALID)";
      std::cout << " <<<" << '\n';
      
      if (diffInt >= 0 && diffInt <= 2) {
        difficulty = static_cast<GameConfig::Difficulty>(diffInt);
        std::cout << "[Server] Parsed difficulty as: " << static_cast<int>(difficulty) << " (" 
                  << (difficulty == GameConfig::Difficulty::EASY ? "EASY" : 
                      difficulty == GameConfig::Difficulty::MEDIUM ? "MEDIUM" : "EXPERT") << ")" << '\n';
      } else {
        std::cout << "[Server] Invalid difficulty value: " << diffInt << ", using default MEDIUM" << '\n';
      }
    } else {
      std::cout << "[Server] >>> NO DIFFICULTY FIELD IN MESSAGE, USING DEFAULT MEDIUM <<<" << '\n';
    }

    lobbyManager.createLobby(lobbyCode, difficulty);
    targetLobby = lobbyManager.getLobby(lobbyCode);
    std::cout << "[Server] Created lobby '" << lobbyCode << "' with final difficulty: " 
              << static_cast<int>(difficulty) << " (" 
              << (difficulty == GameConfig::Difficulty::EASY ? "EASY" : 
                  difficulty == GameConfig::Difficulty::MEDIUM ? "MEDIUM" : "EXPERT") << ")" << '\n';
  }

  // Try to join the lobby
  if (lobbyManager.joinLobby(lobbyCode, clientId)) {
    std::cout << "[Server] Client " << clientId << " joined lobby " << lobbyCode << '\n';
    sendLobbyResponse(clientId, {"lobby_joined", lobbyCode});

    // Notify client about current lobby state
    if (targetLobby != nullptr) {
      nlohmann::json lobbyState;
      lobbyState["type"] = "lobby_state";
      lobbyState["code"] = lobbyCode;
      lobbyState["player_count"] = targetLobby->getClientCount();
      sendJsonMessage(clientId, lobbyState);

      // Notify all other players in the lobby about the new player count
      for (const auto &playerId : targetLobby->getClients()) {
        if (playerId != clientId) {
          sendJsonMessage(playerId, lobbyState);
        }
      }
    }
  } else {
    std::cerr << "[Server] Client " << clientId << " failed to join lobby " << lobbyCode << '\n';
    sendErrorResponse(clientId, "Failed to join lobby");
  }
}

// ============================================================================
// Network Messaging Helpers - All messages are serialized with Cap'n Proto
// ============================================================================

void NetworkReceiveSystem::sendJsonMessage(std::uint32_t clientId, const nlohmann::json &message)
{
  const std::string jsonStr = message.dump();
  const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), clientId);
}

void NetworkReceiveSystem::sendJsonMessageToAll(const std::vector<std::uint32_t> &clientIds,
                                                const nlohmann::json &message)
{
  const std::string jsonStr = message.dump();
  const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  for (const auto &client_id : clientIds) {
    m_networkManager->send(
      std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), client_id);
  }
}

void NetworkReceiveSystem::sendLobbyResponse(std::uint32_t clientId, const LobbyResponseData &response_data)
{
  nlohmann::json response;
  response["type"] = response_data.response_type;
  if (!response_data.lobby_code.empty()) {
    response["code"] = response_data.lobby_code;
  }
  sendJsonMessage(clientId, response);
}

void NetworkReceiveSystem::sendErrorResponse(std::uint32_t clientId, const std::string &error)
{
  nlohmann::json response;
  response["type"] = "error";
  response["message"] = error;
  sendJsonMessage(clientId, response);
}

// ============================================================================
// Lobby Management
// ============================================================================

void NetworkReceiveSystem::handleLeaveLobby([[maybe_unused]] ecs::World &world, std::uint32_t clientId)
{
  std::cout << "[Server] Client " << clientId << " requested to leave lobby" << '\n';

  if (m_game == nullptr) {
    return;
  }

  auto &lobbyManager = m_game->getLobbyManager();

  // Get lobby info before making any changes
  Lobby *lobby = lobbyManager.getClientLobby(clientId);
  if (lobby == nullptr) {
    std::cout << "[Server] Client " << clientId << " is not in any lobby" << '\n';
    return;
  }

  // Capture necessary info before removing client
  const std::string lobbyCode = lobby->getCode();
  std::vector<std::uint32_t> remainingClients;

  // Remove client from lobby (this destroys the player entity)
  lobby->removeClient(clientId);
  std::cout << "[Server] Removed client " << clientId << " from lobby " << lobbyCode << '\n';

  // Check if lobby is now empty
  const bool isLobbyEmpty = lobby->isEmpty();

  if (isLobbyEmpty) {
    std::cout << "[Server] Lobby " << lobbyCode << " is now empty, will be cleaned up at end of frame" << '\n';
  } else {
    // Capture remaining clients for notification
    for (const auto &playerId : lobby->getClients()) {
      remainingClients.push_back(playerId);
    }
  }

  // Remove client from lobby manager tracking
  lobbyManager.leaveLobby(clientId);

  // Notify remaining players about the updated player count
  if (!remainingClients.empty()) {
    // Notify remaining players about the updated player count
    // Must get lobby again after leaveLobby
    Lobby *updatedLobby = lobbyManager.getLobby(lobbyCode);
    if (updatedLobby != nullptr) {
      nlohmann::json lobbyState;
      lobbyState["type"] = "lobby_state";
      lobbyState["code"] = lobbyCode;
      lobbyState["player_count"] = updatedLobby->getClientCount();

      for (const auto &playerId : remainingClients) {
        sendJsonMessage(playerId, lobbyState);
      }
    }
  }

  // Confirm leave to client
  nlohmann::json response;
  response["type"] = "lobby_left";
  sendJsonMessage(clientId, response);
}

void NetworkReceiveSystem::handleSetDifficulty(const nlohmann::json &json, std::uint32_t clientId)
{
  if (!json.contains("difficulty")) {
    std::cerr << "[Server] set_difficulty message from client " << clientId << " has no 'difficulty' field" << '\n';
    return;
  }

  std::string diffStr = json["difficulty"].get<std::string>();
  Difficulty diff;
  if (diffStr == "easy") {
    diff = Difficulty::EASY;
  } else if (diffStr == "medium") {
    diff = Difficulty::MEDIUM;
  } else if (diffStr == "expert") {
    diff = Difficulty::EXPERT;
  } else {
    std::cerr << "[Server] Invalid difficulty from client " << clientId << ": " << diffStr << '\n';
    return;
  }

  if (m_game == nullptr) {
    return;
  }

  auto *lobby = m_game->getLobbyManager().getClientLobby(clientId);
  if (lobby == nullptr) {
    sendErrorResponse(clientId, "Not in any lobby");
    return;
  }

  // Convert Difficulty to GameConfig::Difficulty
  GameConfig::Difficulty gameConfigDiff;
  if (diff == Difficulty::EASY) {
    gameConfigDiff = GameConfig::Difficulty::EASY;
  } else if (diff == Difficulty::MEDIUM) {
    gameConfigDiff = GameConfig::Difficulty::MEDIUM;
  } else {
    gameConfigDiff = GameConfig::Difficulty::EXPERT;
  }

  lobby->setDifficulty(gameConfigDiff);
}
