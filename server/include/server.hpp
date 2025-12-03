/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/

#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "GameMessage.capnp.h"
#include <array>
#include <asio.hpp>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <chrono>
#include <iostream>
#include <kj/array.h>
#include <thread>
#include <vector>
#define UNUSED __attribute__((unused))
#pragma once

class Server
{
  public:
    Server();
    Server(const int port);
    ~Server();

  private:
    std::unique_ptr<NetworkManager> _networkManager;
};
