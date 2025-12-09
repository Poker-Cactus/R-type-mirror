/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioClient.cpp
*/

#include "../include/AsioClient.hpp"
#include "../include/CapnpHandler.hpp"

AsioClient::AsioClient(const std::string &host, const std::string &port)
    : ANetworkManager(std::make_shared<CapnpHandler>()), m_strand(asio::make_strand(m_ioContext)),
      m_socket(m_ioContext), m_workGuard(asio::make_work_guard(m_ioContext))
{
  try {
    asio::ip::udp::resolver resolver(m_ioContext);
    asio::ip::udp::endpoint serverEndpoint = *resolver.resolve(asio::ip::udp::v4(), host, port).begin();

    m_socket.open(asio::ip::udp::v4());
    m_serverEndpoint = serverEndpoint;
  } catch (const std::exception &e) {
    std::cerr << "[Client] Init error: " << e.what() << std::endl;
  }
}

AsioClient::~AsioClient()
{
  stop();
}

void AsioClient::start()
{
  m_recvThread = std::thread([this]() { m_ioContext.run(); });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  receive();
}

void AsioClient::stop()
{
  m_ioContext.stop();
  m_workGuard.reset();
  if (m_recvThread.joinable())
    m_recvThread.join();
}

void AsioClient::send(std::span<const std::byte> data, UNUSED const std::uint32_t &targetEndpointId)
{
  m_socket.async_send_to(
    asio::buffer(data.data(), data.size()), m_serverEndpoint,
    asio::bind_executor(m_strand, [this](const std::error_code &error, std::size_t bytesTransferred) {
      if (error) {
        std::cerr << "[Client] Send error: " << error.message() << std::endl;
      } else {
        std::cout << "[Client] Sent " << bytesTransferred << " bytes" << std::endl;
      }
    }));
}

void AsioClient::receive()
{
  auto recvBuffer = std::make_shared<std::array<char, BUFFER_SIZE>>();

  m_socket.async_receive_from(
    asio::buffer(*recvBuffer), m_serverEndpoint,
    asio::bind_executor(m_strand, [this, recvBuffer](const std::error_code &error, std::size_t bytesTransferred) {
      if (!error && bytesTransferred > 0) {
        std::cout << "[Client] Received " << bytesTransferred << " bytes" << std::endl;
        try {
          const std::string data = getPacketHandler()->deserialize(*recvBuffer, bytesTransferred);
          std::cout << "[Client] Deserialized message: " << data << std::endl;
          std::vector<std::byte> bytes = CapnpHandler::stringToBytes(data);
          NetworkPacket messageQueue(bytes, 0);
          m_incomingMessages.push(messageQueue);

          receive();
        } catch (const std::exception &e) {
          std::cerr << "[Client] Deserialization error: " << e.what() << std::endl;
          receive();
        }
      } else if (error) {
        std::cerr << "[Client] Receive error: " << error.message() << std::endl;
      }
    }));
}

bool AsioClient::poll(NetworkPacket &msg)
{
  return m_incomingMessages.pop(msg);
}
