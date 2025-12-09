/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <memory>

class Server
{
  public:
    Server(std::shared_ptr<INetworkManager> networkManager);
    ~Server();

  private:
    std::shared_ptr<INetworkManager> _networkManager;
};
