/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Client.cpp
*/

#include "../include/Client.hpp"
#include <csignal>

std::atomic<bool> Client::g_running{true};

Client::Client(std::shared_ptr<INetworkManager> networkManager) : m_networkManager(networkManager)
{
  std::signal(SIGINT, Client::signalHandler);
  if (!networkManager) {
    std::cout << "Invalid Network Manager provided to Client." << std::endl;
    return;
  }
  m_networkManager->start();
}

Client::~Client() {}

void Client::loop()
{
  while (g_running) {
    NetworkPacket msg;
    while (m_networkManager->poll(msg)) {
      std::string data = m_networkManager->getPacketHandler()->deserialize(msg.getData(), msg.getBytesTransferred());
      std::cout << "Data: " << data << std::endl;
    }
    auto serialized = m_networkManager->getPacketHandler()->serialize("PING");
    m_networkManager->send(
      std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  auto serialized = m_networkManager->getPacketHandler()->serialize("PING");
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

void Client::signalHandler(int signum)
{
  (void)signum;
  g_running = false;
}
