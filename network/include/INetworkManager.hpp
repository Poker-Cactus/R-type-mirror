/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** INetworkManager.hpp
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class INetworkManager
{
  public:
    virtual ~INetworkManager() = default;

    virtual void run() = 0;
    virtual void send(const std::string &data, const uint32_t &targetEndpointId) = 0;
};
