/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Server.cpp
*/

#include "../include/Server.hpp"
#include <iostream>

Server::Server(std::shared_ptr<INetworkManager> networkManager) : m_networkManager(networkManager)
{
    if (!networkManager) {
        std::cout << "Invalid Network Manager provided to Server." << std::endl;
        return;
    }
    networkManager->start();
}

void Server::run(const std::atomic<bool> &running)
{
    while (running) {
        NetworkPacket msg;
        if (m_networkManager->poll(msg)) {
            std::string data(reinterpret_cast<const char *>(msg.getData().data()), msg.getData().size());
            std::cout << "Data: " << data << std::endl;
        }
    }
}

Server::~Server() {}
