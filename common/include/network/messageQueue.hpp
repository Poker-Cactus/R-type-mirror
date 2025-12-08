/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** messageQueue.hpp
*/

#pragma once

#include <cstdint>
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
    MessageQueue(const std::string &msg, uint32_t senderEndpointId) : _msg(msg), _senderEndpointId(senderEndpointId)
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
    std::string getFullMessage() const { return _msg; }
    uint32_t getSenderEndpointId() const { return _senderEndpointId; }

  private:
    std::uint8_t _header;
    std::string _data;
    std::string _msg;
    uint32_t _senderEndpointId;
};
// }
