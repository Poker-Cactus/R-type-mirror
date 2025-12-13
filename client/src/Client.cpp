/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Client.cpp
*/

#include "../include/Client.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../include/systems/NetworkReceiveSystem.hpp"
#include "../include/systems/NetworkSendSystem.hpp"
#include <chrono>
#include <csignal>

std::atomic<bool> Client::g_running{true};

Client::Client(std::shared_ptr<INetworkManager> networkManager)
    : m_networkManager(networkManager), m_world(std::make_shared<ecs::World>())
{
  std::signal(SIGINT, Client::signalHandler);
  if (!networkManager) {
    std::cout << "Invalid Network Manager provided to Client." << std::endl;
    return;
  }

  // Initialize network manager
  std::cout << "[Client] Starting network manager..." << std::endl;
  m_networkManager->start();
  std::cout << "[Client] Network manager started." << std::endl;

  // Setup ECS systems
  initializeSystems();
}

Client::~Client() {}

void Client::initializeSystems()
{
  // Register network systems
  m_world->registerSystem<NetworkSendSystem>(m_networkManager);
  m_world->registerSystem<ClientNetworkReceiveSystem>(m_networkManager);

  std::cout << "✓ Client: Systèmes réseau initialisés" << std::endl;
  std::cout << "⏳ Client: En attente des entités du serveur..." << std::endl;

  //send packet to the server to notify client is ready
  auto serialized = m_networkManager->getPacketHandler()->serialize("PING");
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
}

void Client::loop()
{
  constexpr float TARGET_FPS = 60.0f;
  constexpr float FRAME_TIME = 1.0f / TARGET_FPS;

  auto lastTime = std::chrono::high_resolution_clock::now();

  std::cout << "🎮 Client: Boucle de jeu démarrée (60 FPS)" << std::endl;

  while (g_running) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    // Update all ECS systems:
    // - ClientNetworkReceiveSystem receives snapshots from server
    // - NetworkSendSystem sends player inputs to server
    m_world->update(deltaTime);

    // Cap to target FPS
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    float frameElapsed = std::chrono::duration<float>(frameEndTime - currentTime).count();

    if (frameElapsed < FRAME_TIME) {
      std::this_thread::sleep_for(std::chrono::duration<float>(FRAME_TIME - frameElapsed));
    }
  }

  std::cout << "🛑 Client: Arrêt de la boucle de jeu" << std::endl;
}

void Client::signalHandler(int signum)
{
  (void)signum;
  g_running = false;
}
