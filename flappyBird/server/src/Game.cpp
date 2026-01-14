/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game.cpp - FlappyBird Server Game Logic Implementation
*/

#include "Game.hpp"
#include "../../../common/include/Common.hpp"
#include "../../../engineCore/include/ecs/EngineComponents.hpp"
#include "systems/NetworkReceiveSystem.hpp"
#include "systems/NetworkSendSystem.hpp"
#include <chrono>
#include <iostream>
#include <thread>

Game::Game()
{
  world = std::make_shared<ecs::World>();

  // Register basic systems for FlappyBird
  world->registerSystem<ecs::MovementSystem>();

  std::cout << "[FlappyBird] Game systems registered" << std::endl;
}

Game::~Game()
{
  running = false;
}

void Game::initializeSystems()
{
  // Initialize any systems that need explicit setup
  std::cout << "[FlappyBird] Systems initialized" << std::endl;
}

void Game::setNetworkManager(const std::shared_ptr<INetworkManager> &networkManager)
{
  m_networkManager = networkManager;

  if (m_networkManager && world) {
    // Register network systems
    m_networkReceiveSystem = &world->registerSystem<NetworkReceiveSystem>(m_networkManager, this);
    m_networkSendSystem = &world->registerSystem<NetworkSendSystem>(m_networkManager);

    std::cout << "[FlappyBird] Network systems registered" << std::endl;
  }
}

std::shared_ptr<ecs::World> Game::getWorld()
{
  return world;
}

void Game::addClient(std::uint32_t clientId)
{
  m_clients.insert(clientId);
  std::cout << "[FlappyBird] Client " << clientId << " connected. Total clients: " << m_clients.size() << std::endl;
}

void Game::removeClient(std::uint32_t clientId)
{
  m_clients.erase(clientId);
  std::cout << "[FlappyBird] Client " << clientId << " disconnected. Total clients: " << m_clients.size() << std::endl;
}

const std::unordered_set<std::uint32_t> &Game::getClients() const
{
  return m_clients;
}

void Game::runGameLoop()
{
  running = true;
  currentTime = std::chrono::steady_clock::now();
  nextTick = currentTime + tickRate;

  std::cout << "[FlappyBird] Game loop started (Tick rate: " << FlappyConfig::TICK_RATE_MS << "ms)" << std::endl;

  while (running) {
    currentTime = std::chrono::steady_clock::now();

    if (currentTime >= nextTick) {
      // Calculate delta time
      auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(tickRate);
      float deltaTime = static_cast<float>(frameDuration.count()) / 1000000.0F;

      // Update all systems
      world->update(deltaTime);

      // Schedule next tick
      nextTick += tickRate;

      // If we're falling behind, skip frames to catch up
      if (currentTime > nextTick) {
        nextTick = currentTime + tickRate;
      }
    } else {
      // Sleep until next tick
      auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(nextTick - currentTime);
      if (sleepTime.count() > 0) {
        std::this_thread::sleep_for(sleepTime);
      }
    }
  }

  std::cout << "[FlappyBird] Game loop ended" << std::endl;
}
