/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioClient.hpp - UDP client using ASIO
*/

#ifndef ASIO_CLIENT_HPP_
#define ASIO_CLIENT_HPP_

#include <asio.hpp>
#include <cstdint>
#include <span>
#include <string>
#include <thread>

#include "../../common/include/network/NetworkPacket.hpp"
#include "../../common/include/network/SafeQueue.hpp"
#include "ANetworkManager.hpp"

/**
 * @brief Asynchronous UDP client using ASIO
 *
 * Connects to a remote server for game communication.
 */
class AsioClient : public ANetworkManager
{
public:
  /**
   * @brief Construct a UDP client
   *
   * @param host The server hostname or IP
   * @param port The server port
   */
  AsioClient(const std::string &host, const std::string &port);
  ~AsioClient();

  void send(std::span<const std::byte> data, const std::uint32_t &targetEndpointId) override;
  void start() override;
  void stop() override;
  bool poll(NetworkPacket &msg) override;
  std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> getClients() const override;

  // Client always talks to a single server endpoint.
  [[nodiscard]] asio::ip::udp::endpoint getServerEndpoint() const { return m_serverEndpoint; }

private:
  void receive();

  SafeQueue<NetworkPacket> m_incomingMessages;
  asio::io_context m_ioContext;
  asio::strand<asio::io_context::executor_type> m_strand;
  asio::ip::udp::socket m_socket;
  asio::ip::udp::endpoint m_serverEndpoint;
  asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
  std::thread m_recvThread;
};

#endif // ASIO_CLIENT_HPP_
