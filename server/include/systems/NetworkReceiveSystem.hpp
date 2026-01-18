/**
 * @file NetworkReceiveSystem.hpp
 * @brief Server-side network receive system.
 */

#ifndef NETWORKReceiveSYSTEM_HPP_
#define NETWORKReceiveSYSTEM_HPP_
#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "../chat/Chat.hpp"
#include <memory>
#include <nlohmann/json.hpp>

class Game;

/**
 * @struct LobbyResponseData
 * @brief Payload for lobby responses sent to clients.
 */
struct LobbyResponseData {
  std::string response_type; ///< Response type string.
  std::string lobby_code; ///< Lobby code associated with the response.
};

/**
 * @class NetworkReceiveSystem
 * @brief Server system handling inbound network messages.
 */
class NetworkReceiveSystem : public ecs::ISystem
{
public:
  /** @brief Construct the receive system. */
  NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager);
  /** @brief Destroy the receive system. */
  ~NetworkReceiveSystem();
  /** @brief Process inbound messages for this frame. */
  void update(ecs::World &world, float deltaTime) override;
  /** @brief Get the component signature for this system. */
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;
  /** @brief Attach the game instance for lobby operations. */
  void setGame(Game *game);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  Game *m_game = nullptr;
  std::unique_ptr<server::Chat> m_chat; ///< Chat system with command handling

  // Message handling
  /** @brief Route an incoming message by type. */
  void handleMessage(ecs::World &world, const std::string &message, std::uint32_t clientId);
  /** @brief Handle player input messages. */
  void handlePlayerInput(ecs::World &world, std::string message, std::uint32_t clientId);
  /** @brief Handle viewport update messages. */
  void handleViewport(ecs::World &world, const nlohmann::json &json, std::uint32_t clientId);
  /** @brief Handle lobby join/create requests. */
  void handleRequestLobby(const nlohmann::json &json, std::uint32_t clientId);
  /** @brief Handle spectator toggle requests. */
  void handleToggleSpectator(const nlohmann::json &json, std::uint32_t clientId);
  /** @brief Handle start-game requests. */
  void handleStartGame(ecs::World &world, std::uint32_t clientId);
  /** @brief Handle leave-lobby requests. */
  void handleLeaveLobby(ecs::World &world, std::uint32_t clientId);
  /** @brief Handle end-screen leave notifications. */
  void handleEndScreenLeft(ecs::World &world, std::uint32_t clientId);
  /** @brief Handle difficulty changes. */
  void handleSetDifficulty(const nlohmann::json &json, std::uint32_t clientId);
  /** @brief Handle chat messages. */
  void handleChatMessage(const nlohmann::json &json, std::uint32_t clientId);

  // Network helpers - properly serialize with Cap'n Proto
  /** @brief Send a JSON message to a client. */
  void sendJsonMessage(std::uint32_t clientId, const nlohmann::json &message);
  /** @brief Send a JSON message to a list of clients. */
  void sendJsonMessageToAll(const std::vector<std::uint32_t> &clientIds, const nlohmann::json &message);
  /** @brief Send a lobby response to a client. */
  void sendLobbyResponse(std::uint32_t clientId, const LobbyResponseData &response_data);
  /** @brief Send an error response to a client. */
  void sendErrorResponse(std::uint32_t clientId, const std::string &error);
};

#endif /* !NETWORKReceiveSYSTEM_HPP_ */
