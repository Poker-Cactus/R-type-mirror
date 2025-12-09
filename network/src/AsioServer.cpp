/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioServer.cpp
*/

#include "../include/AsioServer.hpp"

AsioServer::AsioServer(std::uint16_t port)
    : ANetworkManager(std::make_shared<CapnpHandler>()), m_strand(asio::make_strand(m_ioContext)),
      m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), m_nextClientId(0),
      m_workGuard(asio::make_work_guard(m_ioContext))
{
}

AsioServer::~AsioServer()
{
  stop();
}

void AsioServer::start()
{
  std::size_t nbOfThreads = std::thread::hardware_concurrency();

  for (std::size_t i = 0; i < nbOfThreads; ++i) {
    m_threadPool.emplace_back([this]() { m_ioContext.run(); });
  }
  receive();
}

void AsioServer::stop()
{
  m_ioContext.stop();
  m_workGuard.reset();
  for (auto &thread : m_threadPool) {
    if (thread.joinable())
      thread.join();
  }
}

std::uint32_t AsioServer::getOrCreateClientId(const asio::ip::udp::endpoint &endpoint)
{
  for (const auto &[id, ep] : m_clients) {
    if (ep == endpoint) {
      return id;
    }
  }
  std::uint32_t clientId = m_nextClientId++;
  m_clients[clientId] = endpoint;
  std::cout << "[Server] New client connected: " << clientId << std::endl;
  return clientId;
}

void AsioServer::send(std::span<const std::byte> data, const std::uint32_t &targetEndpointId)
{
  auto targetEndpointIt = m_clients.find(targetEndpointId);

  if (targetEndpointIt == m_clients.end()) {
    std::cerr << "[Server] Client ID not found: " << targetEndpointId << std::endl;
    return;
  }
  m_socket.async_send_to(
    asio::buffer(data.data(), data.size()), targetEndpointIt->second,
    asio::bind_executor(m_strand, [this, targetEndpointId](const std::error_code &error, std::size_t bytesTransferred) {
      if (error) {
        std::cerr << "[Server] Send error: " << error.message() << std::endl;
      } else {
        std::cout << "[Server] Sent " << bytesTransferred << " bytes to client " << targetEndpointId << std::endl;
      }
    }));
}

void AsioServer::receive()
{
  auto recvBuffer = std::make_shared<std::array<char, BUFFER_SIZE>>();

  m_socket.async_receive_from(
    asio::buffer(*recvBuffer), m_remoteEndpoint,
    asio::bind_executor(m_strand, [this, recvBuffer](const std::error_code &error, std::size_t bytesTransferred) {
      if (!error && bytesTransferred > 0) {
        std::cout << "[Server] Received " << bytesTransferred << " bytes" << std::endl;
        try {
          std::string data = m_packetHandler->deserialize(*recvBuffer, bytesTransferred);
          std::cout << "[Server] Deserialized message: " << data << std::endl;
          std::uint32_t clientId = getOrCreateClientId(m_remoteEndpoint);
          std::vector<std::byte> bytes = CapnpHandler::stringToBytes(data);
          NetworkPacket messageQueue(bytes, clientId);
          m_incomingMessages.push(messageQueue);

          receive();
        } catch (const std::exception &e) {
          std::cerr << "[Server] Deserialization error: " << e.what() << std::endl;
          receive();
        }
      } else if (error) {
        std::cerr << "[Server] Receive error: " << error.message() << std::endl;
      }
    }));
}

bool AsioServer::poll(NetworkPacket &msg)
{
  return m_incomingMessages.pop(msg);
}
