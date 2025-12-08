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
        MessageQueue msg;
        if (_networkManager->poll(msg)) {
            std::cout << "Data: " << msg.getData() << std::endl;
        }
        auto serialized = _networkManager->getPacketHandler()->serialize("PING");
        _networkManager->send(
            std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
    }
}

Client::~Client() {}
