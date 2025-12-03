/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/server.hpp"

Server::Server() : _networkManager(std::make_unique<NetworkManager>(4242))
{
    _networkManager->run();
}

Server::Server(const int port) : _networkManager(std::make_unique<NetworkManager>(port)) {}

Server::~Server() {}
