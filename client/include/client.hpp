/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** client.hpp
*/

#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include <array>
#include <asio.hpp>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <iostream>
#include <thread>
#include "GameMessages.capnp.h"
#define UNUSED __attribute__((unused))
#pragma once

class UdpClient
{
  public:
    UdpClient(const std::string &host, const std::string &port);
    ~UdpClient();

    void send(const std::string &msg);
    bool getIncomingMessage(std::string &msg);
    void recvLoop();
    void loop();

  private:
    void startReceive();

    asio::io_context _io_context;
    std::thread _network_thread;
    bool _running;
    asio::ip::udp::socket _socket;
    asio::strand<asio::io_context::executor_type> _strand;
    std::array<char, 1024> _recv_buffer;
    SafeQueue<std::string> _inComingMessages;
};
