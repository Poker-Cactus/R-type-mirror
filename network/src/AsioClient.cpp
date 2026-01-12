/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioClient.cpp
*/

#include "../include/AsioClient.hpp"
#include "../include/CapnpHandler.hpp"
#include "ANetworkManager.hpp"
#include "Common.hpp"
#include "network/NetworkPacket.hpp"
#include <array>
#include <asio/bind_executor.hpp>
#include <asio/buffer.hpp>
#include <asio/error.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/ip/udp.hpp>
#include <asio/strand.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <thread>

AsioClient::AsioClient(const std::string &host, const std::string &port)
    : ANetworkManager(std::make_shared<CapnpHandler>()), m_strand(asio::make_strand(m_ioContext)),
      m_socket(m_ioContext), m_workGuard(asio::make_work_guard(m_ioContext)), m_statsResetTime(std::chrono::steady_clock::now())
{
  try {
    asio::ip::udp::resolver resolver(m_ioContext);
    asio::ip::udp::endpoint serverEndpoint = *resolver.resolve(asio::ip::udp::v4(), host, port).begin();

    m_socket.open(asio::ip::udp::v4());
    m_serverEndpoint = serverEndpoint;
    m_connected = true; // Assume connected initially
  } catch (const std::exception &e) {
    std::cerr << "[Client] Init error: " << e.what() << std::endl;
    m_connected = false;
  }
}

AsioClient::~AsioClient()
{
  stop();
}

void AsioClient::start()
{
  receive();
  m_recvThread = std::thread([this]() { m_ioContext.run(); });
}

void AsioClient::stop()
{
  m_ioContext.stop();
  m_workGuard.reset();
  if (m_recvThread.joinable()) {
    m_recvThread.join();
  }
}

void AsioClient::send(std::span<const std::byte> data, UNUSED const std::uint32_t &targetEndpointId)
{
  // Track upload stats
  m_uploadByteCount += static_cast<int>(data.size());
  m_packetCount++; // Assuming each send is a packet

  m_socket.async_send_to(
    asio::buffer(data.data(), data.size()), m_serverEndpoint,
    asio::bind_executor(m_strand, [this](const std::error_code &error, UNUSED std::size_t bytesTransferred) {
      if (error) {
        std::cerr << "[Client] Send error: " << error.message() << std::endl;
        m_connected = false;
      } else {
        m_connected = true;
        // std::cout << "[Client] Sent " << bytesTransferred << " bytes" << std::endl;
      }
    }));
}

void AsioClient::receive()
{
  auto buffer = std::make_shared<std::array<char, BUFFER_SIZE>>();
  auto senderEndpoint = std::make_shared<asio::ip::udp::endpoint>();

  m_socket.async_receive_from(
    asio::buffer(*buffer), *senderEndpoint,
    asio::bind_executor(m_strand,
                        [this, buffer, senderEndpoint](const std::error_code &error, std::size_t bytesTransferred) {
                          if (!error || error != asio::error::operation_aborted) {
                            receive();
                          }
                          if (!error && bytesTransferred > 0) {
                            // Track download stats
                            m_downloadByteCount += static_cast<int>(bytesTransferred);
                            m_packetCount++; // Assuming each receive is a packet

                            try {
                              NetworkPacket message(*buffer, 0, static_cast<std::uint32_t>(bytesTransferred));
                              m_incomingMessages.push(message);

                              // Check for pong response
                              auto deserialized = getPacketHandler()->deserialize(*buffer, bytesTransferred);
                              if (deserialized && *deserialized == "PONG" && m_pingPending) {
                                auto now = std::chrono::steady_clock::now();
                                m_latency = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pingStartTime).count());
                                m_pingPending = false;
                              }
                            } catch (const std::exception &e) {
                              std::cerr << "[Client] Deserialization error: " << e.what() << std::endl;
                            }
                          } else if (error) {
                            std::cerr << "[Client] Receive error: " << error.message() << std::endl;
                            m_connected = false;
                          }
                        }));
}

bool AsioClient::poll(NetworkPacket &msg)
{
  return m_incomingMessages.pop(msg);
}

std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> AsioClient::getClients() const
{
  return {};
}

float AsioClient::getLatency() const
{
  return m_latency;
}

bool AsioClient::isConnected() const
{
  return m_connected;
}

int AsioClient::getPacketsPerSecond() const
{
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_statsResetTime).count();
  if (elapsed >= 1) {
    m_packetsPerSecond = static_cast<int>(m_packetCount / elapsed);
    m_uploadBytesPerSecond = static_cast<int>(m_uploadByteCount / elapsed);
    m_downloadBytesPerSecond = static_cast<int>(m_downloadByteCount / elapsed);
    m_packetCount = 0;
    m_uploadByteCount = 0;
    m_downloadByteCount = 0;
    m_statsResetTime = now;
  }
  return m_packetsPerSecond;
}

int AsioClient::getUploadBytesPerSecond() const
{
  // Already updated in getPacketsPerSecond
  getPacketsPerSecond();
  return m_uploadBytesPerSecond;
}

int AsioClient::getDownloadBytesPerSecond() const
{
  // Already updated in getPacketsPerSecond
  getPacketsPerSecond();
  return m_downloadBytesPerSecond;
}

void AsioClient::sendPing()
{
  if (!m_pingPending) {
    m_pingStartTime = std::chrono::steady_clock::now();
    m_pingPending = true;
    auto serialized = getPacketHandler()->serialize("PING");
    send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
  }
}
