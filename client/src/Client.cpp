/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Client.cpp
*/

#include "../include/Client.hpp"

Client::Client(std::shared_ptr<INetworkManager> networkManager) : m_networkManager(networkManager)
{
    if (!networkManager) {
        std::cout << "Invalid Network Manager provided to Client." << std::endl;
        return;
    }
    m_networkManager->start();
}

void Client::loop(const std::atomic<bool> &running)
{
    while (running) {
        NetworkPacket msg;
        while (m_networkManager->poll(msg)) {
            std::string data(reinterpret_cast<const char *>(msg.getData().data()), msg.getData().size());
            std::cout << "Data: " << data << std::endl;
        }
        auto serialized = m_networkManager->getPacketHandler()->serialize("PING");
        m_networkManager->send(
            std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

Client::~Client() {}
