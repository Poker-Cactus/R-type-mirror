/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IPacketHandler.hpp - Packet serialization interface
*/

#ifndef I_PACKET_HANDLER_HPP_
#define I_PACKET_HANDLER_HPP_

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../../common/include/Common.hpp"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <kj/std/iostream.h>

/**
 * @brief Interface for packet serialization/deserialization
 *
 * Defines contract for converting between message strings and byte arrays.
 */
class IPacketHandler
{
public:
  virtual ~IPacketHandler() = default;

  /**
   * @brief Serialize a message string to bytes
   *
   * @param data The message string
   * @return Vector of serialized bytes
   */
  virtual std::vector<std::uint8_t> serialize(const std::string &data) const = 0;

  /**
   * @brief Deserialize bytes to a message string
   *
   * @param buffer The receive buffer
   * @param bytesTransferred Number of bytes received
   * @return Deserialized message string
   */
  virtual std::string deserialize(const std::array<char, BUFFER_SIZE> &buffer, std::size_t bytesTransferred) const = 0;
};

#endif // I_PACKET_HANDLER_HPP_
