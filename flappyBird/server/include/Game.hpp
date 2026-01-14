/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game.hpp - FlappyBird Server Game Logic
*/

#pragma once

#include "../../../engineCore/include/ecs/World.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <unordered_set>

// FlappyBird game configuration
namespace FlappyConfig
{
constexpr int DEFAULT_PORT = 4243;
constexpr int TICK_RATE_MS = 16; // ~60 FPS
constexpr float GRAVITY = 980.0F;
constexpr float JUMP_VELOCITY = -400.0F;
constexpr float BIRD_SPAWN_X = 100.0F;
constexpr float BIRD_SPAWN_Y = 300.0F;
} // namespace FlappyConfig

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
  std::shared_ptr<ecs::World> getWorld();

  void addClient(std::uint32_t clientId);
  void removeClient(std::uint32_t clientId);
  [[nodiscard]] const std::unordered_set<std::uint32_t> &getClients() const;

private:
  std::shared_ptr<ecs::World> world;
  std::shared_ptr<INetworkManager> m_networkManager;

  NetworkReceiveSystem *m_networkReceiveSystem = nullptr;
  NetworkSendSystem *m_networkSendSystem = nullptr;

  bool running = false;
  std::chrono::steady_clock::time_point currentTime;
  std::chrono::steady_clock::time_point nextTick;
  std::chrono::milliseconds tickRate{FlappyConfig::TICK_RATE_MS};

  std::unordered_set<std::uint32_t> m_clients;
};
