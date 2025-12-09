/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.hpp
*/
#pragma once

#include "IPacketHandler.hpp"

class CapnpHandler : public IPacketHandler
{
  public:
    CapnpHandler() = default;
    ~CapnpHandler() override = default;
    std::vector<uint8_t> serialize(const std::string &data) const override;
    std::string deserialize(const std::array<char, BUFFER_SIZE> &recvBuffer,
                            const std::size_t bytes_transferred) const override;
    static std::vector<std::byte> stringToBytes(const std::string &str);
};
