/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/

#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include <array>
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#define UNUSED __attribute__((unused))
#pragma once

class UdpServer
{
  public:
    UdpServer(short port);
    ~UdpServer();

    void asyncSend(const std::string &message, asio::ip::udp::endpoint target_endpoint);
    bool getIncomingMessage(MessageQueue &message);

    void run();

    const asio::ip::udp::socket &getSocket() const { return _socket; }

  private:
    void startReceive();

    bool _running;

    asio::io_context _io_context;
    asio::executor_work_guard<asio::io_context::executor_type> _work_guard;

    std::vector<std::thread> _threadPool;
    asio::ip::udp::socket _socket;
    asio::strand<asio::io_context::executor_type> _strand;
    asio::ip::udp::endpoint _remote_endpoint; // Stocke qui nous a envoyé le dernier paquet
    std::array<char, 1024> _recv_buffer; // Buffer de réception
    SafeQueue<MessageQueue> _inComingMessages;
};
