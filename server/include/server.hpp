/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/

#include "../../network/include/networkManager.hpp"
#include "GameMessages.capnp.h"
#include <memory>
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
