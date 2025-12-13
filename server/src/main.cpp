/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "Game.hpp"
#include <iostream>
#include <thread>

int main()
{
  std::cout << "🎮 R-Type Server Starting..." << std::endl;

  try {
    Game game;
    std::cout << "Game initialized with all systems" << std::endl;

    // Run game in separate thread for now
    std::thread gameThread([&game]() { game.runGameLoop(); });

    std::cout << "Press Ctrl+C to stop server" << std::endl;

    gameThread.join();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
