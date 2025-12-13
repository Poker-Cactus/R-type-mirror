/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../../engineCore/include/ecs/events/GameEvents.hpp"
#include "../../network/include/AsioServer.hpp"
#include "Game.hpp"
#include <iostream>
#include <thread>

int main()
{
  std::cout << "🎮 R-Type Server Starting..." << std::endl;

  try {
    auto networkManager = std::make_shared<AsioServer>(4242);

    Game game;
    std::cout << "Game initialized with all systems" << std::endl;

    game.setNetworkManager(networkManager);

    // Create some initial entities via spawn events
    auto world = game.getWorld();
    if (world) {
      networkManager->setWorld(world);
      networkManager->start();

      std::cout << "Spawning initial entities..." << std::endl;
      // Spawn two enemies at different Y positions
      world->emitEvent(ecs::SpawnEntityEvent(ecs::SpawnEntityEvent::EntityType::ENEMY, 800.0F, 200.0F));
      world->emitEvent(ecs::SpawnEntityEvent(ecs::SpawnEntityEvent::EntityType::ENEMY, 800.0F, 350.0F));

      // Spawn a projectile (no explicit spawner)
      world->emitEvent(ecs::SpawnEntityEvent(ecs::SpawnEntityEvent::EntityType::PROJECTILE, 150.0F, 300.0F, 0));
    }

    // Run game in separate thread for now
    std::thread gameThread([&game]() { game.runGameLoop(); });

    std::cout << "Press Ctrl+C to stop server" << std::endl;

    gameThread.join();

    std::cout << "Total connected players handled: " << networkManager->getConnectedPlayersCount() << std::endl;
    networkManager->stop();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
