/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** messageQueue.hpp
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

class NetworkPacket
{
  public:
    NetworkPacket() = default;
    NetworkPacket(std::vector<std::byte> data, uint32_t senderEndpointId)
        : _data(std::move(data)), _senderEndpointId(senderEndpointId)
    {
    }

    ~NetworkPacket() = default;

    const std::vector<std::byte> &getData() const { return _data; }
    uint32_t getSenderEndpointId() const { return _senderEndpointId; }

  private:
    std::vector<std::byte> _data;
    uint32_t _senderEndpointId;
};
// }
