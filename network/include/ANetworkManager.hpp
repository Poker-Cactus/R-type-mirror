/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ANetworkManagr.hpp
*/

#pragma once

#include "INetworkManager.hpp"
#include "IPacketHandler.hpp"

class ANetworkManager : public INetworkManager
{
  public:
    ANetworkManager(std::shared_ptr<IPacketHandler> packetHandler);
    virtual ~ANetworkManager() = default;

    std::shared_ptr<IPacketHandler> getPacketHandler() const override { return _packetHandler; }

  protected:
    std::shared_ptr<IPacketHandler> _packetHandler;
};
