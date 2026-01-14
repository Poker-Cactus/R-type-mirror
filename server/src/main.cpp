/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../../network/include/AsioServer.hpp"
#include "../include/config/ShipStatsConfig.hpp"
#include "Game.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

int main()
{
  std::cout << "🎮 R-Type Server Starting..." << '\n';

  // Load ship stats configuration
  auto &shipStatsConfig = server::ShipStatsConfig::getInstance();
  if (!shipStatsConfig.loadFromFile("server/config/ship_stats.json")) {
    std::cerr << "⚠️  Warning: Failed to load ship stats config, using defaults" << '\n';
  }

  try {
    auto networkManager = std::make_shared<AsioServer>(GameConfig::DEFAULT_PORT);

    Game game;
    std::cout << "Game initialized with all systems" << '\n';

    game.setNetworkManager(networkManager);

    auto world = game.getWorld();
    if (world) {
      networkManager->setWorld(world);
      networkManager->start();
    }

    std::thread gameThread([&game]() { game.runGameLoop(); });

    std::cout << "Press Ctrl+C to stop server" << '\n';

    gameThread.join();

    networkManager->stop();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
