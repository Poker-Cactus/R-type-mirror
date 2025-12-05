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
    networkManager->run();
}

Client::~Client() {}
