/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/systems/MovementSystem.hpp"
#include <chrono>
#include <thread>

class Game
{
public:
  Game();
  ~Game();
  void runGameLoop();
  std::shared_ptr<ecs::World> getWorld();

private:
  std::shared_ptr<ecs::World> world;
  bool running = false;
  std::chrono::steady_clock::time_point currentTime;
  std::chrono::steady_clock::time_point nextTick;
  std::chrono::milliseconds tickRate{16};
};

#endif /* !GAME_HPP_ */
