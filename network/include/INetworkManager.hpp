/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** INetworkManager.hpp
*/

#include <vector>
#pragma once

class INetworkManager
{
  public:
    virtual ~INetworkManager() = default;

    virtual void run() = 0;
    template <typename T, typename U> virtual void send(const T &data, U targetEndpoint) = 0;
    virtual void serialize(const std::string &data) = 0;
};
