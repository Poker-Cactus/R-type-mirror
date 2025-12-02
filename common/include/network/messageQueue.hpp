/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** messageQueue.hpp
*/

#include <asio.hpp>
#include <iostream>
#include <string>
#pragma once

typedef enum MessageHeader_s : std::uint8_t {
    MESSAGE_PING = 0x0,
    MESSAGE_PONG = 0x1,
} MessageHeader_t;

// namespace Network {
class MessageQueue
{
  public:
    MessageQueue() = default;
    MessageQueue(const std::string &msg, asio::ip::udp::endpoint _remote_endpoint)
    {
        if (!msg.empty()) {
            _header = static_cast<std::uint8_t>(msg[0]);
            if (_header == MESSAGE_PING || _header == MESSAGE_PONG)
                _data = msg.substr(1);
        }
        _sender_endpoint = _remote_endpoint;
    }

    ~MessageQueue() = default;

    std::uint8_t getHeader() const { return _header; }
    std::string getData() const { return _data; }
    asio::ip::udp::endpoint getSenderEndpoint() const { return _sender_endpoint; }

  private:
    std::uint8_t _header = 0;
    std::string _data;
    asio::ip::udp::endpoint _sender_endpoint;
};
// }