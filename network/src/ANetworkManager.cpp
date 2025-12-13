/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ANetworkManager.cpp
*/

#include "../include/ANetworkManager.hpp"

ANetworkManager::ANetworkManager(std::shared_ptr<IPacketHandler> packetHandler) : m_packetHandler(packetHandler) {}
