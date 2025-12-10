/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#include "Game.hpp"

Game::Game()
{
  world = std::make_shared<ecs::World>();
  world->registerSystem<ecs::MovementSystem>();
}

Game::~Game() {}

void Game::runGameLoop()
{
  running = true;
  nextTick = std::chrono::steady_clock::now();

  while (running) {
    currentTime = std::chrono::steady_clock::now();

    if (currentTime < nextTick) {
      std::this_thread::sleep_for(nextTick - currentTime);
    }
    world->update(0.016f);
    nextTick += tickRate;
  }
}

std::shared_ptr<ecs::World> Game::getWorld()
{
  return world;
}
