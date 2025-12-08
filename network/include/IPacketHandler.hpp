/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IPacketHandler.hpp
*/

#pragma once

#include "../../common/include/common.hpp"
#include "GameMessage.capnp.h"
#include <array>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <kj/std/iostream.h>
#include <span>
#include <string>
#include <vector>

class IPacketHandler
{
  public:
    virtual ~IPacketHandler() = default;

    virtual std::vector<uint8_t> serialize(const std::string &data) const = 0;
    virtual std::string deserialize(const std::array<char, BUFFER_SIZE> &recvBuffer,
                                    const std::size_t bytes_transferred) const = 0;
};
