/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_
#include "../../engineCore/include/ecs/World.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/ShootingSystem.hpp"
#include "systems/SpawnSystem.hpp"
#include <chrono>
#include <cstdint>
#include <memory>

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

private:
  std::shared_ptr<ecs::World> world;

  std::shared_ptr<INetworkManager> m_networkManager;
  NetworkReceiveSystem *m_networkReceiveSystem = nullptr;
  NetworkSendSystem *m_networkSendSystem = nullptr;
  bool running = false;
  std::chrono::steady_clock::time_point currentTime;
  std::chrono::steady_clock::time_point nextTick;
  std::chrono::milliseconds tickRate{16};

  // System pointers for initialization
  server::DamageSystem *damageSystem = nullptr;
  server::DeathSystem *deathSystem = nullptr;
  server::ShootingSystem *shootingSystem = nullptr;
  server::SpawnSystem *spawnSystem = nullptr;
};

#endif /* !GAME_HPP_ */
