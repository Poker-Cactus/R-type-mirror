/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** client.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <iostream>
#include <memory>
#include <thread>

class Client
{
  public:
    Client(std::shared_ptr<INetworkManager> networkManager);
    ~Client();

    void loop();

  private:
    std::shared_ptr<INetworkManager> _networkManager;
};
