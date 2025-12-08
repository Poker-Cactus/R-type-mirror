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
        NetworkPacket msg;
        if (networkManager->poll(msg)) {
            std::string data(reinterpret_cast<const char *>(msg.getData().data()), msg.getData().size());
            std::cout << "Data: " << data << std::endl;
        }
    }
}

Server::~Server() {}
