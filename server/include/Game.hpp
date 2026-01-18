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

// Reference resolution (base for normalization)
constexpr float REFERENCE_WIDTH = 1920.0F;
constexpr float REFERENCE_HEIGHT = 1080.0F;

// Player configuration (normalized to reference resolution)
constexpr float PLAYER_GUN_OFFSET = 20.0F;
constexpr float PLAYER_SPAWN_X = 100.0F;  // ~5.2% of width
constexpr float PLAYER_SPAWN_Y = 300.0F;  // ~27.8% of height
constexpr int PLAYER_MAX_HP = 100;
constexpr float PLAYER_COLLIDER_WIDTH = 140.0F;
constexpr float PLAYER_COLLIDER_HEIGHT = 60.0F;
constexpr int PLAYER_SPRITE_WIDTH = 140;
constexpr int PLAYER_SPRITE_HEIGHT = 60;

// Ally configuration (normalized to reference resolution)
constexpr float ALLY_SPAWN_X = 150.0F;  // ~7.8% of width
constexpr float ALLY_SPAWN_Y = 350.0F;  // ~32.4% of height
constexpr int ALLY_MAX_HP = 100;
// Ally collider dimensions (box) — prefer width/height like player
constexpr float ALLY_COLLIDER_WIDTH = 32.0F;
constexpr float ALLY_COLLIDER_HEIGHT = 32.0F;
// Backwards-compat alias for code using single size constant
constexpr float ALLY_COLLIDER_SIZE = 32.0F;
constexpr int ALLY_SPRITE_WIDTH = 140;
constexpr int ALLY_SPRITE_HEIGHT = 60;

// Projectile collider dimensions (commonly used sizes)
constexpr float PROJECTILE_COLLIDER_WIDTH = 18.0F;
constexpr float PROJECTILE_COLLIDER_HEIGHT = 14.0F;
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
  // Map collision initialization removed (feature temporarily disabled)

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
  // ecs::Entity m_mapEntity = 0; // Entity holding map collision data (removed)
};

#endif /* !GAME_HPP_ */
