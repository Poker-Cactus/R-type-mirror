/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkPacket.hpp - Network packet data structure
*/

#ifndef NETWORK_PACKET_HPP_
#define NETWORK_PACKET_HPP_

#include "../Common.hpp"
#include <array>
#include <cstdint>
#include <vector>

/**
 * @brief Data structure for network messages
 *
 * Contains the serialized message data and sender endpoint ID.
 */
class NetworkPacket
{
public:
  NetworkPacket() = default;

  /**
   * @brief Construct a network packet
   *
   * @param data Message data as bytes
   * @param senderEndpointId ID of the sending endpoint
   */
  NetworkPacket(const std::array<char, BUFFER_SIZE> &data, std::uint32_t senderEndpointId)
      : m_data(data), m_senderEndpointId(senderEndpointId)
  {
  }

  /**
   * @brief Construct a network packet
   *
   * @param data Message data as bytes
   * @param senderEndpointId ID of the sending endpoint
   * @param bytesTransferred Number of bytes transferred
   */
  NetworkPacket(const std::array<char, BUFFER_SIZE> &data, std::uint32_t senderEndpointId,  
                std::uint32_t bytesTransferred)
      : m_data(data), m_senderEndpointId(senderEndpointId), m_bytesTransferred(bytesTransferred)
  {
  }

  ~NetworkPacket() = default;

  /**
   * @brief Get the message data
   *
   * @return Const reference to the data template
   */
  std::array<char, BUFFER_SIZE> getData() const { return m_data; }

  /**
   * @brief Set the message data
   *
   * @param data The data to set
   */
  void setData(const std::array<char, BUFFER_SIZE> &data) { m_data = data; }

  /**
   * @brief Get the sender endpoint ID
   *
   * @return The endpoint ID
   */
  std::uint32_t getSenderEndpointId() const { return m_senderEndpointId; }

  /**
   * @brief Get the number of bytes transferred
   *
   * @return The bytes transferred
   */
  std::uint32_t getBytesTransferred() const { return m_bytesTransferred; }

  /**
   * @brief Set the number of bytes transferred
   *
   * @param bytesTransferred The bytes transferred to set
   */
  void setBytesTransferred(std::uint32_t bytesTransferred) { m_bytesTransferred = bytesTransferred; }

private:
  std::array<char, BUFFER_SIZE> m_data;
  std::uint32_t m_senderEndpointId{0};
  std::uint32_t m_bytesTransferred{0};
};

#endif // NETWORK_PACKET_HPP_
