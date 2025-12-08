/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** asioClient.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "../../common/include/network/NetworkPacket.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "ANetworkManager.hpp"
#include "GameMessage.capnp.h"
#include "capnpHandler.hpp"
#include <array>
#include <asio.hpp>
#include <iostream>
#include <thread>

class AsioClient : public ANetworkManager
{
  public:
    AsioClient(const std::string &host, const std::string &port);
    ~AsioClient();

    void send(std::span<const std::byte> data, const uint32_t &targetEndpointId) override;
    void start() override;
    void stop() override;
    bool poll(NetworkPacket &msg);

  private:
    void recv();

    SafeQueue<NetworkPacket> _inComingMessages;
    asio::io_context _ioContext;
    asio::strand<asio::io_context::executor_type> _strand;
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _serverEndpoint;
    asio::executor_work_guard<asio::io_context::executor_type> _workGuard;
    std::thread _recvThread;
};
