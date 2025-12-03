/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** INetworkManager.hpp
*/

#pragma once

class INetworkManager
{
  public:
    virtual ~INetworkManager() = default;

    virtual void run() = 0;
    template <typename T> virtual void send(std::vector<T> &data, asio::ip::udp::endpoint target_endpoint) = 0;
    virtual void send(const T &data, asio::ip::udp::endpoint target_endpoint) = 0;
    // virtual void send(kj::Array<capnp::word> &data, asio::ip::udp::endpoint target_endpoint) = 0;
    virtual void send(kj::Array<T> &data, asio::ip::udp::endpoint target_endpoint) = 0;
};
