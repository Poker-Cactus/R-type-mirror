/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** INetworkManager.hpp
*/

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

class INetworkManager
{
  public:
    virtual ~INetworkManager() = default;

    virtual void run() = 0;
    virtual void send(std::span<const std::byte> data, const uint32_t &targetEndpointId) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};
