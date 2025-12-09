/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** INetworkManager.hpp - Network manager interface
*/

#ifndef I_NETWORK_MANAGER_HPP_
#define I_NETWORK_MANAGER_HPP_

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "../../common/include/network/NetworkPacket.hpp"
#include "IPacketHandler.hpp"

/**
 * @brief Network event types
 */
enum class EventType { DATA, CONNECT, DISCONNECT };

/**
 * @brief Network event structure
 */
struct NetworkEvent {
    EventType type;
    std::vector<std::byte> data;
    std::uint32_t endpointId;
};

/**
 * @brief Interface for network manager implementations
 *
 * Defines contract for UDP communication and packet handling.
 */
class INetworkManager
{
  public:
    virtual ~INetworkManager() = default;

    /**
     * @brief Send data to a target endpoint
     *
     * @param data The data to send
     * @param targetEndpointId The target endpoint ID
     */
    virtual void send(std::span<const std::byte> data, const std::uint32_t &targetEndpointId) = 0;

    /**
     * @brief Start the network manager
     */
    virtual void start() = 0;

    /**
     * @brief Stop the network manager
     */
    virtual void stop() = 0;

    /**
     * @brief Poll for incoming packets
     *
     * @param msg Packet to populate
     * @return true if a packet was received
     */
    virtual bool poll(NetworkPacket &msg) = 0;

    /**
     * @brief Get the packet handler
     *
     * @return Shared pointer to the packet handler
     */
    virtual std::shared_ptr<IPacketHandler> getPacketHandler() const = 0;
};

#endif // I_NETWORK_MANAGER_HPP_
