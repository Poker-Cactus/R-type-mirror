/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** asioClient.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "GameMessage.capnp.h"
#include "INetworkManager.hpp"
#include "packetHandler.hpp"
#include <array>
#include <asio.hpp>
#include <iostream>
#include <thread>

class AsioClient : public INetworkManager
{
  public:
    static constexpr size_t BUFFER_SIZE = 1024;

    AsioClient(const std::string &host, const std::string &port);
    ~AsioClient();

    void run() override;
    void send(std::span<const std::byte> data, const uint32_t &targetEndpointId) override;
    void start() override;
    void stop() override;

  private:
    void startReceive();
    bool getIncomingMessage(MessageQueue &message);
    std::string buildMessage(uint8_t header, const std::string &data) const;

    SafeQueue<MessageQueue> _inComingMessages;
    asio::io_context _ioContext;
    asio::strand<asio::io_context::executor_type> _strand;
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _serverEndpoint;
    bool _running;
    asio::executor_work_guard<asio::io_context::executor_type> _workGuard;
    std::thread _recvThread;
};
