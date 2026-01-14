/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp - FlappyBird Server Entry Point
*/

#include "../../../network/include/AsioServer.hpp"
#include "Game.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

int main()
{
  std::cout << "🐦 FlappyBird Server Starting..." << '\n';

  try {
    // Use port 4243 to avoid conflict with R-Type server
    auto networkManager = std::make_shared<AsioServer>(4243);

    Game game;
    std::cout << "FlappyBird game initialized" << '\n';

    game.setNetworkManager(networkManager);

    auto world = game.getWorld();
    if (world) {
      networkManager->setWorld(world);
      networkManager->start();
    }

    std::thread gameThread([&game]() { game.runGameLoop(); });

    std::cout << "FlappyBird server running on port 4243" << '\n';
    std::cout << "Press Ctrl+C to stop server" << '\n';

    gameThread.join();

    networkManager->stop();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
