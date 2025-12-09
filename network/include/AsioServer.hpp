/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioServer.hpp - UDP server using ASIO
*/

#ifndef ASIO_SERVER_HPP_
#define ASIO_SERVER_HPP_

#include <asio.hpp>
#include <cstdint>
#include <iostream>
#include <span>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../../common/include/Common.hpp"
#include "../../common/include/network/NetworkPacket.hpp"
#include "../../common/include/network/SafeQueue.hpp"
#include "ANetworkManager.hpp"
#include "CapnpHandler.hpp"
#include "GameMessage.capnp.h"

/**
 * @brief Asynchronous UDP server using ASIO
 *
 * Handles multiple clients with a thread pool.
 */
class AsioServer : public ANetworkManager
{
public:
  /**
   * @brief Construct a UDP server
   *
   * @param port The port to listen on
   */
  explicit AsioServer(std::uint16_t port);
  ~AsioServer();

  void send(std::span<const std::byte> data, const std::uint32_t &targetEndpointId) override;
  void start() override;
  void stop() override;
  bool poll(NetworkPacket &msg) override;

private:
  void receive();
  std::uint32_t getOrCreateClientId(const asio::ip::udp::endpoint &endpoint);

  SafeQueue<NetworkPacket> m_incomingMessages;
  asio::io_context m_ioContext;
  asio::strand<asio::io_context::executor_type> m_strand;
  asio::ip::udp::socket m_socket;
  std::vector<std::thread> m_threadPool;
  std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> m_clients;
  std::uint32_t m_nextClientId;
  asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
  asio::ip::udp::endpoint m_remoteEndpoint;
};

#endif // ASIO_SERVER_HPP_
