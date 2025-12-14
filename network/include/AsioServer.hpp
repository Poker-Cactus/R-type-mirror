/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioServer.hpp - UDP server using ASIO
*/

#ifndef ASIO_SERVER_HPP_
#define ASIO_SERVER_HPP_

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#endif

#include <asio.hpp>
#include <cstdint>
#include <memory>
#include <span>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../common/include/network/NetworkPacket.hpp"
#include "../../common/include/network/SafeQueue.hpp"
#include "ANetworkManager.hpp"

namespace ecs
{
class World;
}

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
  [[nodiscard]] std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> getClients() const override;
  void setWorld(const std::shared_ptr<ecs::World> &world);
  [[nodiscard]] std::size_t getConnectedPlayersCount() const;

private:
  void receive();
  std::pair<std::uint32_t, bool> getOrCreateClientId(const asio::ip::udp::endpoint &endpoint);
  void createPlayerEntity(std::uint32_t clientId);

  SafeQueue<NetworkPacket> m_incomingMessages;
  asio::io_context m_ioContext;
  asio::strand<asio::io_context::executor_type> m_strand;
  asio::ip::udp::socket m_socket;
  std::vector<std::thread> m_threadPool;
  std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> m_clients;
  std::uint32_t m_nextClientId;
  asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
  asio::ip::udp::endpoint m_remoteEndpoint;
  std::shared_ptr<ecs::World> m_world;
  std::size_t m_connectedPlayersCount{0};
};

#endif // ASIO_SERVER_HPP_
