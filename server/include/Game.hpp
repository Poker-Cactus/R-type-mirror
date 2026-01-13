/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_
#include "../../common/include/Common.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "Difficulty.hpp"
#include "LobbyManager.hpp"
#include "ServerSystems.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <unordered_set>

namespace server
{
class EnemyConfigManager;
class LevelConfigManager;
} // namespace server

// Game configuration constants
namespace GameConfig
{
// Server settings
constexpr int DEFAULT_PORT = 4242;
constexpr int TICK_RATE_MS = 16;
constexpr float MICROSECONDS_TO_SECONDS = 1000000.0F;

// Player configuration
constexpr float PLAYER_GUN_OFFSET = 20.0F;
constexpr float PLAYER_SPAWN_X = 100.0F;
constexpr float PLAYER_SPAWN_Y = 300.0F;
constexpr int PLAYER_MAX_HP = 100;
constexpr float PLAYER_COLLIDER_SIZE = 32.0F;
constexpr int PLAYER_SPRITE_WIDTH = 140;
constexpr int PLAYER_SPRITE_HEIGHT = 60;
} // namespace GameConfig

class NetworkReceiveSystem;
class NetworkSendSystem;
class INetworkManager;

class Game
{
public:
  Game();
  ~Game();
  void setNetworkManager(const std::shared_ptr<INetworkManager> &networkManager);
  void runGameLoop();
  void initializeSystems();
  void spawnPlayer();
  void spawnPlayer(std::uint32_t networkId);
  std::shared_ptr<ecs::World> getWorld();
  void startGame();
  [[nodiscard]] bool isGameStarted() const;
  void addClientToLobby(std::uint32_t clientId);
  void removeClientFromLobby(std::uint32_t clientId);
  [[nodiscard]] const std::unordered_set<std::uint32_t> &getLobbyClients() const;
  [[nodiscard]] LobbyManager &getLobbyManager();

  Difficulty currentDifficulty = Difficulty::MEDIUM;

private:
  std::shared_ptr<ecs::World> world;

  std::shared_ptr<INetworkManager> m_networkManager;
  NetworkReceiveSystem *m_networkReceiveSystem = nullptr;
  NetworkSendSystem *m_networkSendSystem = nullptr;
  bool running = false;
  bool gameStarted = false;
  std::chrono::steady_clock::time_point currentTime;
  std::chrono::steady_clock::time_point nextTick;
  std::chrono::milliseconds tickRate{GameConfig::TICK_RATE_MS};

  // System pointers for initialization
  server::DamageSystem *damageSystem = nullptr;
  server::DeathSystem *deathSystem = nullptr;
  server::ShootingSystem *shootingSystem = nullptr;
  server::ScoreSystem *scoreSystem = nullptr;
  server::PowerupSystem *powerupSystem = nullptr;
  server::SpawnSystem *spawnSystem = nullptr;

  std::shared_ptr<server::EnemyConfigManager> m_enemyConfigManager;
  std::shared_ptr<server::LevelConfigManager> m_levelConfigManager;

  std::unordered_set<std::uint32_t> m_lobbyClients;
  LobbyManager m_lobbyManager;
};

#endif /* !GAME_HPP_ */
