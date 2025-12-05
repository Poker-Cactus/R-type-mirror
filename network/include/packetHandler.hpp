/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/
#pragma once

#include "GameMessage.capnp.h"
#include <array>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <kj/std/iostream.h>
#include <span>
#include <string>
#include <vector>

class PacketHandler
{
  public:
    static std::vector<uint8_t> serialize(const std::string &data);
    static std::string deserialize(const std::array<char, 1024> &recvBuffer, const std::size_t bytes_transferred);
    static std::span<const std::byte> stringToBytes(const std::string &str);
};
