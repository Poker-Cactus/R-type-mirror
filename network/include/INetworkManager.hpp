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
#include <asio.hpp>

/**
 * @brief Network event types
 */
enum class EventType : std::uint8_t { DATA, CONNECT, DISCONNECT };

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

  virtual std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> getClients() const = 0;

  /**
   * @brief Disconnect a client by ID (server only)
   * @param clientId The client ID to disconnect
   */
  virtual void disconnect(std::uint32_t clientId) = 0;

  /**
   * @brief Get current network latency in milliseconds
   * @return Latency in ms, or -1 if not available
   */
  virtual float getLatency() const = 0;

  /**
   * @brief Check if client is connected to server
   * @return true if connected
   */
  virtual bool isConnected() const = 0;

  /**
   * @brief Get packets per second (received)
   * @return Packets per second
   */
  virtual int getPacketsPerSecond() const = 0;

  /**
   * @brief Get upload bandwidth in bytes per second
   * @return Upload bytes per second
   */
  virtual int getUploadBytesPerSecond() const = 0;

  /**
   * @brief Get download bandwidth in bytes per second
   * @return Download bytes per second
   */
  virtual int getDownloadBytesPerSecond() const = 0;
};

#endif // I_NETWORK_MANAGER_HPP_
