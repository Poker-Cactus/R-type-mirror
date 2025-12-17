/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** CapnpHandler.hpp - Cap'n Proto packet handler
*/

#ifndef CAPNP_HANDLER_HPP_
#define CAPNP_HANDLER_HPP_

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "IPacketHandler.hpp"

/**
 * @brief Cap'n Proto implementation of packet serialization
 *
 * Handles serialization and deserialization using Cap'n Proto format.
 */
class CapnpHandler : public IPacketHandler
{
public:
  CapnpHandler() = default;
  ~CapnpHandler() override = default;

  /**
   * @brief Serialize a message string
   *
   * @param data The message string
   * @return Serialized bytes
   */
  std::vector<std::uint8_t> serialize(const std::string &data) const override;

  /**
   * @brief Deserialize bytes to string
   *
   * @param buffer The receive buffer
   * @param bytesTransferred Number of bytes received
   * @return Deserialized message string
   */
  std::optional<std::string> deserialize(const std::array<char, BUFFER_SIZE> &buffer,
                                         std::size_t bytesTransferred) const override;

  /**
   * @brief Convert string to byte vector
   *
   * @param str The string to convert
   * @return Vector of bytes
   */
  static std::vector<std::byte> stringToBytes(const std::string &str);
};

#endif // CAPNP_HANDLER_HPP_
