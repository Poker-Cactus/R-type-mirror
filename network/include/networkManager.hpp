/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/

#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "INetworkManager.hpp"
#include <array>
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <vector>

#define UNUSED __attribute__((unused))
#pragma once

class NetworkManager ::INetworkManager
{
  public:
    NetworkManager(short port);
    ~NetworkManager();

    void run();
    void send(const std::string &message, asio::ip::udp::endpoint target_endpoint);
    void send(kj::Array<capnp::word> &data, asio::ip::udp::endpoint target_endpoint);

  private:
    void startReceive();
    bool getIncomingMessage(MessageQueue &message);

    const asio::ip::udp::socket &getSocket() const { return _socket; }

    bool _running;

    asio::io_context _io_context;
    asio::executor_work_guard<asio::io_context::executor_type> _work_guard;

    std::vector<std::thread> _threadPool;
    asio::ip::udp::socket _socket;
    asio::strand<asio::io_context::executor_type> _strand;
    asio::ip::udp::endpoint _remote_endpoint; // Stocke qui nous a envoyé le dernier paquet
    std::array<char, 1024> _recv_buffer;
    SafeQueue<MessageQueue> _inComingMessages;
};
