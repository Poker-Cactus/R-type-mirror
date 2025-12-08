/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** INetworkManager.hpp
*/

#pragma once

#include "../../common/include/network/messageQueue.hpp"
#include "IPacketHandler.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

typedef struct NetworkEvent_s {
    enum EventType { DATA, CONNECT, DICONNECT } type;
    std::vector<std::byte> data;
    uint32_t endpointId;
} NetworkEvent_t;

class INetworkManager
{
  public:
    virtual ~INetworkManager() = default;

    virtual void send(std::span<const std::byte> data, const uint32_t &targetEndpointId) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool poll(MessageQueue &msg) = 0;
    virtual std::shared_ptr<IPacketHandler> getPacketHandler() const = 0;
};
