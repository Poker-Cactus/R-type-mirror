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
#include <memory>

class Game
{
public:
  Game();
  ~Game();
  void runGameLoop();
  void initializeSystems();
  void spawnPlayer();
  std::shared_ptr<ecs::World> getWorld();

private:
  std::shared_ptr<ecs::World> world;
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
