/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Server.cpp
*/

#include "../include/Server.hpp"
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

std::atomic<bool> Server::g_running{true};

Server::Server(std::shared_ptr<INetworkManager> networkManager) : m_networkManager(networkManager)
{
  std::signal(SIGINT, Server::signalHandler);
  if (!networkManager) {
    std::cout << "Invalid Network Manager provided to Server." << std::endl;
    return;
  }
  networkManager->start();
}

Server::~Server() {}

void Server::loop()
{
  while (g_running) {
    NetworkPacket msg;
    if (m_networkManager->poll(msg)) {
      auto data = m_networkManager->getPacketHandler()->deserialize(msg.getData(), msg.getBytesTransferred());
      if (!data.has_value()) {
        std::cout << "Failed to deserialize incoming packet." << std::endl;
        continue;
      }
      std::cout << "Data: " << data.value() << std::endl;
    }
    auto serialized = m_networkManager->getPacketHandler()->serialize("PONG");
    m_networkManager->send(
      std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void Server::signalHandler(int signum)
{
  (void)signum;
  g_running = false;
}
