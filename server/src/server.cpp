/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/server.hpp"
#include <iostream>

Server::Server(std::shared_ptr<INetworkManager> networkManager) : _networkManager(networkManager)
{
    if (!networkManager) {
        std::cout << "Invalid Network Manager provided to Server." << std::endl;
        return;
    }
    networkManager->start();
    while (true) {
        MessageQueue msg;
        if (networkManager->poll(msg)) {
            std::cout << "Data: " << msg.getData() << std::endl;
        }
    }
}

Server::~Server() {}
