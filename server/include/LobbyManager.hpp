/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LobbyManager - Manages multiple game lobbies
*/

#ifndef LOBBY_MANAGER_HPP_
#define LOBBY_MANAGER_HPP_

#include "Lobby.hpp"
#include "../../common/include/Common.hpp"
#include <memory>
#include <string>
#include <unordered_map>

#include "../../network/include/INetworkManager.hpp"

namespace server
{
class EnemyConfigManager;
class LevelConfigManager;
} // namespace server

/**
 * @brief Manages creation and access to game lobbies
 */
class LobbyManager
{
public:
  LobbyManager() = default;
  ~LobbyManager() = default;

  void setNetworkManager(std::shared_ptr<INetworkManager> networkManager)
  {
    m_networkManager = std::move(networkManager);
  }

  void setEnemyConfigManager(std::shared_ptr<server::EnemyConfigManager> configManager)
  {
    m_enemyConfigManager = configManager;
  }

  void setLevelConfigManager(std::shared_ptr<server::LevelConfigManager> configManager)
  {
    m_levelConfigManager = configManager;
  }

  /**
   * @brief Create a new lobby with a unique code and specified difficulty
   * @param code The lobby code
   * @param difficulty The game difficulty
   * @return true if lobby was created, false if code already exists
   */
  bool createLobby(const std::string &code, GameConfig::Difficulty difficulty = GameConfig::Difficulty::MEDIUM,
                   bool isSolo = false, AIDifficulty aiDifficulty = AIDifficulty::MEDIUM,
                   GameMode mode = GameMode::CLASSIC);

  /**
   * @brief Add a client to a lobby
   * @param code The lobby code
   * @param clientId The client identifier
   * @param asSpectator Whether to join as spectator
   * @return true if client was added successfully
   */
  bool joinLobby(const std::string &code, std::uint32_t clientId, bool asSpectator = false);

  /**
   * @brief Remove a client from their current lobby
   * @param clientId The client identifier
   */
  void leaveLobby(std::uint32_t clientId);

  /**
   * @brief Get the lobby a client is in
   * @param clientId The client identifier
   * @return Pointer to lobby or nullptr if not in any lobby
   */
  [[nodiscard]] Lobby *getClientLobby(std::uint32_t clientId);

  /**
   * @brief Get a lobby by code
   * @param code The lobby code
   * @return Pointer to lobby or nullptr if not found
   */
  [[nodiscard]] Lobby *getLobby(const std::string &code);

  /**
   * @brief Remove empty lobbies
   */
  void cleanupEmptyLobbies();

  /**
   * @brief Get all active lobbies
   * @return Map of lobby codes to lobby instances
   */
  [[nodiscard]] const std::unordered_map<std::string, std::unique_ptr<Lobby>> &getLobbies() const;

private:
  std::unordered_map<std::string, std::unique_ptr<Lobby>> m_lobbies;
  std::unordered_map<std::uint32_t, std::string> m_clientToLobby;
  std::shared_ptr<INetworkManager> m_networkManager;
  std::shared_ptr<server::EnemyConfigManager> m_enemyConfigManager;
  std::shared_ptr<server::LevelConfigManager> m_levelConfigManager;
};

#endif /* !LOBBY_MANAGER_HPP_ */
