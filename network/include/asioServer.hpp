/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** asioServer.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "ANetworkManager.hpp"
#include "GameMessage.capnp.h"
#include "capnpHandler.hpp"
#include "messageQueue.hpp"
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <unordered_map>

class AsioServer : public ANetworkManager
{
  public:
    AsioServer(const short &port);
    ~AsioServer();

    void recv() override;
    void send(std::span<const std::byte> data, const uint32_t &targetEndpointId) override;
    void start() override;
    void stop() override;
    bool poll(MessageQueue &msg);

  private:
    void registerClient(const asio::ip::udp::endpoint &endpoint);
    std::string buildMessage(uint8_t header, const std::string &data) const;

    SafeQueue<MessageQueue> _inComingMessages;
    asio::io_context _ioContext;
    asio::strand<asio::io_context::executor_type> _strand;
    asio::ip::udp::socket _socket;
    std::vector<std::thread> _threadPool;
    std::unordered_map<uint32_t, asio::ip::udp::endpoint> _clients;
    uint32_t _nextClientId = 0;
    bool _running;
    asio::executor_work_guard<asio::io_context::executor_type> _workGuard;
    asio::ip::udp::endpoint _remoteEndpoint;
    // std::array<std::byte, BUFFER_SIZE> _recvBuffer;
};
