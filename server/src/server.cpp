/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/server.hpp"

Server::Server(std::shared_ptr<INetworkManager> networkManager) : _networkManager(networkManager)
{
    if (networkManager)
        networkManager->run();
}

Server::~Server() {}
