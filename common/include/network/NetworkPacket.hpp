/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkPacket.hpp - Network packet data structure
*/

#ifndef NETWORK_PACKET_HPP_
#define NETWORK_PACKET_HPP_

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
    NetworkPacket(std::vector<std::byte> data, std::uint32_t senderEndpointId)
        : m_data(std::move(data)), m_senderEndpointId(senderEndpointId)
    {
    }

    ~NetworkPacket() = default;

    /**
     * @brief Get the message data
     *
     * @return Const reference to the data vector
     */
    const std::vector<std::byte> &getData() const { return m_data; }

    /**
     * @brief Get the sender endpoint ID
     *
     * @return The endpoint ID
     */
    std::uint32_t getSenderEndpointId() const { return m_senderEndpointId; }

  private:
    std::vector<std::byte> m_data;
    std::uint32_t m_senderEndpointId{0};
};

#endif // NETWORK_PACKET_HPP_
