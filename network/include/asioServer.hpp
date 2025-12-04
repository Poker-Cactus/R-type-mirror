/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** asioServer.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "GameMessage.capnp.h"
#include "INetworkManager.hpp"
#include "packetHandler.hpp"
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <unordered_map>

class AsioServer : public INetworkManager
{
  public:
    static constexpr size_t BUFFER_SIZE = 1024;

    AsioServer(const short &port);
    ~AsioServer();

    void run() override;
    void send(const std::string &data, const uint32_t &targetEndpointId) override;

  private:
    void startReceive();
    bool getIncomingMessage(MessageQueue &message);
    uint32_t registerOrGetClient(const asio::ip::udp::endpoint &endpoint);
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
};
