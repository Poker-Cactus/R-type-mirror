/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** client.cpp
*/

#include "../include/client.hpp"

Client::Client(std::shared_ptr<INetworkManager> networkManager) : _networkManager(networkManager)
{
    if (!networkManager) {
        std::cout << "Invalid Network Manager provided to Client." << std::endl;
        return;
    }
    _networkManager->start();
}

void Client::loop()
{
    while (true) {
        NetworkPacket msg;
        while (_networkManager->poll(msg)) {
            std::string data(reinterpret_cast<const char *>(msg.getData().data()), msg.getData().size());
            std::cout << "Data: " << data << std::endl;
        }
        auto serialized = _networkManager->getPacketHandler()->serialize("PING");
        _networkManager->send(
            std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

Client::~Client() {}
