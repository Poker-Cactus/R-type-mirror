/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/

#include "../../common/include/network/messageQueue.hpp"
#include "../../common/include/network/safeQueue.hpp"
#include "../../network/include/networkManager.hpp"
#include "GameMessages.capnp.h"
#include <array>
#include <asio.hpp>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <iostream>
#include <memory>
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
