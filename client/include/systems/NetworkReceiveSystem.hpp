/**
 * @file NetworkReceiveSystem.hpp
 * @brief Client-side network message reception system
 */

#ifndef CLIENT_NETWORKRECEIVESYSTEM_HPP_
#define CLIENT_NETWORKRECEIVESYSTEM_HPP_

#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <functional>
#include <nlohmann/json.hpp>
#include <string>

/**
 * @class ClientNetworkReceiveSystem
 * @brief Handles incoming network messages from the server
 *
 * This system processes messages including:
 * - Entity creation/updates
 * - Game state snapshots
 * - Lobby events
 * - Game lifecycle events
 */
class ClientNetworkReceiveSystem : public ecs::ISystem
{
public:
  /**
   * @brief Construct the network receive system
   * @param networkManager Shared pointer to network manager
   */
  ClientNetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager);
  ~ClientNetworkReceiveSystem() override;

  /**
   * @brief Update system - process incoming network messages
   * @param world ECS world reference
   * @param deltaTime Time elapsed since last update
   */
  void update(ecs::World &world, float deltaTime) override;

  /**
   * @brief Get component signature for this system
   * @return Component signature
   */
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

  /**
   * @brief Set callback for game start event
   * @param callback Function to call when game starts
   */
  void setGameStartedCallback(std::function<void()> callback);

  /**
   * @brief Set callback for lobby join event
   * @param callback Function to call when joining lobby
   */
  void setLobbyJoinedCallback(std::function<void(const std::string &)> callback);

  /**
   * @brief Set callback for lobby state updates
   * @param callback Function to call on lobby state change
   */
  void setLobbyStateCallback(std::function<void(const std::string &, int, int)> callback);

  /**
   * @brief Set callback for error events
   * @param callback Function to call on error
   */
  void setErrorCallback(std::function<void(const std::string &)> callback);

  /**
   * @brief Enable or disable snapshot processing
   * @param accept Whether to accept game snapshots
   */
  void setAcceptSnapshots(bool accept);

  /**
   * @brief Set callback for lobby leave event
   * @param callback Function to call when leaving lobby
   */
  void setLobbyLeftCallback(std::function<void()> callback);
  /**
   * @brief Set callback for lobby messages (temporary notices)
   * @param callback Function to call when a lobby message is received (message, durationSeconds)
   */
  void setLobbyMessageCallback(std::function<void(const std::string &, int)> callback);

  /**
   * @brief Set callback for player death event
   * @param callback Function to call when player dies
   */
  void setPlayerDeadCallback(std::function<void(const nlohmann::json &)> callback);

  /**
   * @brief Set callback for chat message event
   * @param callback Function to call when chat message received (sender, content, senderId)
   */
  void setChatMessageCallback(std::function<void(const std::string &, const std::string &, std::uint32_t)> callback);
  /**
   * @brief Set callback for lobby end event (scores)
   */
  void setLobbyEndCallback(std::function<void(const nlohmann::json &)> callback);

  /**
   * @brief Set callback for level complete event
   * @param callback Function to call when level is completed (currentLevel, nextLevel)
   */
  void setLevelCompleteCallback(std::function<void(const std::string &, const std::string &)> callback);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  std::function<void()> m_gameStartedCallback;
  std::function<void(const std::string &)> m_lobbyJoinedCallback;
  std::function<void(const std::string &, int, int)> m_lobbyStateCallback;
  std::function<void(const std::string &)> m_errorCallback;
  std::function<void()> m_lobbyLeftCallback;
  std::function<void(const nlohmann::json &)> m_playerDeadCallback;
  std::function<void(const std::string &, const std::string &, std::uint32_t)> m_chatMessageCallback;
  std::function<void(const std::string &, const std::string &)> m_levelCompleteCallback;
  std::function<void(const std::string &, int)> m_lobbyMessageCallback;
  std::function<void(const nlohmann::json &)> m_lobbyEndCallback;

  /** @brief Handle entity creation from a network message. */
  void handleEntityCreated(ecs::World &world, const nlohmann::json &json);
  /** @brief Handle entity update from a network message. */
  void handleEntityUpdate(ecs::World &world, const nlohmann::json &json);
  /** @brief Handle a snapshot update from the server. */
  void handleSnapshot(ecs::World &world, const nlohmann::json &json);
  /** @brief Trigger the game-start callback. */
  void handleGameStarted();
};

#endif /* !CLIENT_NETWORKRECEIVESYSTEM_HPP_ */
