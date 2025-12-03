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
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <iostream>
#include <thread>

#pragma once
#define UNUSED __attribute__((unused))

class NetworkManager ::INetworkManager
{
  public:
    NetworkManager(short port);
    ~NetworkManager();

    void run();
    template <typename T, typename U> void send(const T &data, U targetEndpoint);

    void serialize(const T &data);
    std::string &deserialize(const std::array<char, 1024> &recvBuffer, const std::size_t bytes_transferred)

        private : void startReceive();
    bool getIncomingMessage(MessageQueue &message);

    const asio::ip::udp::socket &getSocket() const { return _socket; }

    bool _running;

    asio::io_context _io_context;
    asio::executor_work_guard<asio::io_context::executor_type> _work_guard;

    std::vector<std::thread> _threadPool;
    asio::ip::udp::socket _socket;
    asio::strand<asio::io_context::executor_type> _strand;
    asio::ip::udp::endpoint _remote_endpoint; // Stocke qui nous a envoyé le dernier paquet
    SafeQueue<MessageQueue> _inComingMessages;
};
