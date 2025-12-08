/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** messageQueue.hpp
*/

#pragma once

#include <asio.hpp>
#include <iostream>
#include <string>

typedef enum MessageHeader_s : std::uint8_t {
    MESSAGE_PING = 0x0,
    MESSAGE_PONG = 0x1,
} MessageHeader_t;

// namespace Network {
class MessageQueue
{
  public:
    MessageQueue() = default;
    MessageQueue(const std::string &msg, asio::ip::udp::endpoint _remoteEndpoint)
        : _fullMessage(msg), _senderEndpoint(_remoteEndpoint)
    {
        if (!msg.empty()) {
            _header = static_cast<std::uint8_t>(msg[0]);
            if (_header == MESSAGE_PING || _header == MESSAGE_PONG)
                _data = msg.substr(1);
            else
                _data = msg;
        }
    }

    ~MessageQueue() = default;

    std::uint8_t getHeader() const { return _header; }
    std::string getData() const { return _data; }
    std::string getFullMessage() const { return _fullMessage; }
    asio::ip::udp::endpoint getSenderEndpoint() const { return _senderEndpoint; }

  private:
    std::uint8_t _header = 0;
    std::string _data;
    std::string _fullMessage;
    asio::ip::udp::endpoint _senderEndpoint;
};
// }
