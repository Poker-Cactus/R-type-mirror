/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Lobby - Manages game lobbies with unique codes and isolated game worlds
*/

#ifndef LOBBY_HPP_
#define LOBBY_HPP_

#include "../../common/include/Common.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "Difficulty.hpp"
#include <nlohmann/json.hpp>

// Forward declarations
class INetworkManager;
namespace server
{
class EnemyConfigManager;
class LevelConfigManager;
} // namespace server
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 * @brief Represents a game lobby with a unique code and isolated game world
 */
class Lobby
{
public:
  explicit Lobby(const std::string &code, std::shared_ptr<INetworkManager> networkManager = nullptr,
                 bool isSolo = false, AIDifficulty aiDifficulty = AIDifficulty::MEDIUM,
                 GameMode mode = GameMode::CLASSIC);
  ~Lobby();

  // Disable copy and move to prevent issues with unique resources
  Lobby(const Lobby &) = delete;
  Lobby &operator=(const Lobby &) = delete;
  Lobby(Lobby &&) = delete;
  Lobby &operator=(Lobby &&) = delete;

  /**
   * @brief Add a client to the lobby
   * @param clientId The client identifier
   * @param asSpectator Whether to add as spectator (no player entity created)
   * @return true if client was added, false if already present
   */
  bool addClient(std::uint32_t clientId, bool asSpectator = false);

  /**
   * @brief Remove a client from the lobby
   * @param clientId The client identifier
   * @return true if client was removed, false if not found
   */
  bool removeClient(std::uint32_t clientId);

  /**
   * @brief Check if a client is a spectator
   * @param clientId The client identifier
   * @return true if client is a spectator
   */
  [[nodiscard]] bool isSpectator(std::uint32_t clientId) const;

  /**
   * @brief Check if lobby is empty
   * @return true if no clients are in the lobby
   */
  [[nodiscard]] bool isEmpty() const;

  /**
   * @brief Get the number of clients in the lobby
   * @return Number of clients
   */
  [[nodiscard]] std::size_t getClientCount() const;

  /**
   * @brief Get the number of players in the lobby (excluding spectators)
   * @return Number of players
   */
  [[nodiscard]] std::size_t getPlayerCount() const;

  /**
   * @brief Get the lobby code
   * @return The unique lobby code
   */
  [[nodiscard]] const std::string &getCode() const;

  /**
   * @brief Get all clients in the lobby
   * @return Set of client IDs
   */
  [[nodiscard]] const std::unordered_set<std::uint32_t> &getClients() const;

  /**
   * @brief Check if a client is in the lobby
   * @param clientId The client identifier
   * @return true if client is present
   */
  [[nodiscard]] bool hasClient(std::uint32_t clientId) const;

  /**
   * @brief Start the game for this lobby - initializes the world and spawns players
   */
  void startGame();

  /**
   * @brief Stop the game for this lobby - clears the world
   */
  void stopGame();

  /**
   * @brief Check if the game has started
   * @return true if game is in progress
   */
  [[nodiscard]] bool isGameStarted() const;

  /**
   * @brief Get the lobby's isolated game world
   * @return Shared pointer to the ECS world
   */
  [[nodiscard]] std::shared_ptr<ecs::World> getWorld() const;

  /**
   * @brief Update the lobby's game world
   * @param deltaTime Time since last update
   */
  void update(float deltaTime);

  /**
   * @brief Get the player entity for a client
   * @param clientId The client identifier
   * @return Entity ID or 0 if not found
   */
  [[nodiscard]] ecs::Entity getPlayerEntity(std::uint32_t clientId) const;

  /**
   * @brief Send a JSON message to a specific client in this lobby
   */
  void sendJsonToClient(std::uint32_t clientId, const nlohmann::json &message) const;

  /**
   * @brief Set the enemy configuration manager for this lobby
   * @param configManager Shared pointer to enemy config manager
   */
  void setEnemyConfigManager(std::shared_ptr<server::EnemyConfigManager> configManager);

  /**
   * @brief Set the level configuration manager for this lobby
   * @param configManager Shared pointer to level config manager
   */
  void setLevelConfigManager(std::shared_ptr<server::LevelConfigManager> configManager);

  /**
   * @brief Set the difficulty for this lobby
   * @param difficulty The game difficulty
   * @note Must be called before startGame() to take effect
   */
  void setDifficulty(GameConfig::Difficulty difficulty);

  /**
   * @brief Get the game difficulty setting
   * @return The game difficulty
   */
  [[nodiscard]] GameConfig::Difficulty getDifficulty() const;

  /**
   * @brief Set the game mode for this lobby
   * @param mode The game mode
   * @note Must be called before startGame() to take effect
   */
  void setGameMode(GameMode mode);

  /**
   * @brief Get the game mode for this lobby
   * @return The game mode
   */
  [[nodiscard]] GameMode getGameMode() const;

  /**
   * @brief Convert a player to spectator after death
   * @param clientId The client identifier
   */
  void convertToSpectator(std::uint32_t clientId);

  /**
   * @brief Trigger end-of-game flow: stop spawning, collect scores and notify clients
   */
  void endGameShowScores();

  /**
   * @brief Notify the lobby that a client has left the end-screen view
   */
  void notifyEndScreenLeft(std::uint32_t clientId);

  [[nodiscard]] bool isEndScreenActive() const { return m_endScreenActive; }

  /**
   * @brief Request that the owning LobbyManager destroy this lobby
   * This will notify the manager to stop and remove the lobby.
   */
  void requestDestroy();

  /**
   * @brief Set owning LobbyManager for callbacks
   */
  void setManager(class LobbyManager *manager);

  /**
   * @brief Convert a spectator back to player (recreate player entity if game started)
   * @param clientId The client identifier
   */
  void convertToPlayer(std::uint32_t clientId);

  /**
   * @brief Get the AI difficulty setting
   * @return The AI difficulty
   */
  [[nodiscard]] AIDifficulty getAIDifficulty() const;

private:
  void initializeSystems();
  // Map collisions removed: no initialization required
  void spawnPlayer(std::uint32_t clientId);
  void spawnAlly();
  void destroyPlayerEntity(std::uint32_t clientId);

  std::string m_code;
  std::unordered_set<std::uint32_t> m_clients;
  std::unordered_set<std::uint32_t> m_spectators;
  bool m_gameStarted = false;
  bool m_isSolo = false;

  // Isolated game world for this lobby
  std::shared_ptr<ecs::World> m_world;

  // Network manager used to send direct messages to clients in this lobby
  std::shared_ptr<INetworkManager> m_networkManager;

  // Map client IDs to their player entities
  std::unordered_map<std::uint32_t, ecs::Entity> m_playerEntities;
  ecs::Entity m_allyEntity = 0;
  // Map collision entity removed
  // Pointer back to owning manager (not owning)
  class LobbyManager *m_manager = nullptr;

  // Enemy configuration manager
  std::shared_ptr<server::EnemyConfigManager> m_enemyConfigManager;

  // Level configuration manager
  std::shared_ptr<server::LevelConfigManager> m_levelConfigManager;

  // Game difficulty setting
  GameConfig::Difficulty m_difficulty = GameConfig::Difficulty::MEDIUM;

  // Game mode setting
  GameMode m_gameMode = GameMode::CLASSIC;

  // AI difficulty setting
  AIDifficulty m_aiDifficulty = AIDifficulty::MEDIUM;

  // End-of-game state: store final scores when player entities are removed
  bool m_endScreenActive = false;
  std::unordered_map<std::uint32_t, int> m_finalScores; // clientId -> score
  std::unordered_set<std::uint32_t> m_endScreenViewers; // clients currently viewing the score page
  // Optional mapping of client IDs to their display names (provided by client)
  std::unordered_map<std::uint32_t, std::string> m_clientNames;

public:
  // Store a client's display name for use in UI (scores, chat, etc.)
  void setClientName(std::uint32_t clientId, const std::string &name);
  [[nodiscard]] std::string getClientName(std::uint32_t clientId) const;
};

#endif /* !LOBBY_HPP_ */
