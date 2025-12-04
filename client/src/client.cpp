/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** client.cpp
*/

#include "../include/client.hpp"

Client::Client(std::shared_ptr<INetworkManager> networkManager) : _networkManager(networkManager)
{
    if (networkManager)
        networkManager->run();
}

Client::~Client() {}
