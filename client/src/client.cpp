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
    networkManager->start();
    while (true) {
        MessageQueue msg;
        if (networkManager->poll(msg)) {
            std::cout << "Data: " << msg.getData() << std::endl;
        }
        auto serialized = _networkManager->getPacketHandler()->serialize("PING");
        auto byteSpan =
            std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size());
        networkManager->send(byteSpan, 0);
    }
}

Client::~Client() {}
