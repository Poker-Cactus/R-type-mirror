/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** asioServer.cpp
*/

#include "../include/asioServer.hpp"

AsioServer::AsioServer(const short &port)
    : ANetworkManager(std::make_shared<CapnpHandler>()), _strand(asio::make_strand(_ioContext)),
      _socket(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), _running(true),
      _workGuard(asio::make_work_guard(_ioContext))
{
}

AsioServer::~AsioServer()
{
    stop();
}

void AsioServer::start()
{
    int nbOfThreads = std::thread::hardware_concurrency();

    for (int i = 0; i < nbOfThreads; i++) {
        _threadPool.emplace_back([this]() { _ioContext.run(); });
    }
    recv();
}

void AsioServer::stop()
{
    _ioContext.stop();
    _workGuard.reset();
    for (auto &t : _threadPool) {
        if (t.joinable())
            t.join();
    }
}

void AsioServer::registerClient(const asio::ip::udp::endpoint &endpoint)
{
    for (const auto &[id, ep] : _clients) {
        if (ep == endpoint) {
            return;
        }
    }
    uint32_t clientId = _nextClientId++;
    _clients[clientId] = endpoint;
    std::cout << "[Server] New client connected: " << clientId << std::endl;
}

std::string AsioServer::buildMessage(uint8_t header, const std::string &data) const
{
    return std::string(1, static_cast<char>(header)) + data;
}

void AsioServer::send(std::span<const std::byte> data, const uint32_t &targetEndpointId)
{
    auto targetEndpointIt = _clients.find(targetEndpointId);

    if (targetEndpointIt == _clients.end()) {
        std::cerr << "[Server] Client ID not found: " << targetEndpointId << std::endl;
        return;
    }
    _socket.async_send_to(
        asio::buffer(data.data(), data.size()), targetEndpointIt->second,
        asio::bind_executor(_strand,
                            [this, targetEndpointId](const std::error_code &error, std::size_t bytes_transferred) {
                                if (error) {
                                    std::cerr << "[Server] Send error: " << error.message() << std::endl;
                                } else {
                                    std::cout << "[Server] Sent " << bytes_transferred << " bytes to client "
                                              << targetEndpointId << std::endl;
                                }
                            }));
}

void AsioServer::recv()
{
    auto recvBuffer = std::make_shared<std::array<char, BUFFER_SIZE>>();

    _socket.async_receive_from(
        asio::buffer(*recvBuffer), _remoteEndpoint,
        asio::bind_executor(_strand, [this, recvBuffer](const std::error_code &error, std::size_t bytes_transferred) {
            if (!error && bytes_transferred > 0) {
                std::cout << "[Server] Received " << bytes_transferred << " bytes" << std::endl;
                try {
                    std::string data = _packetHandler->deserialize(*recvBuffer, bytes_transferred);
                    std::cout << "[Server] Deserialized message: " << data << std::endl;
                    MessageQueue messageQueue(data, _remoteEndpoint);
                    _inComingMessages.push(messageQueue);
                    registerClient(_remoteEndpoint);

                    recv();
                } catch (const std::exception &e) {
                    std::cerr << "[Server] Deserialization error: " << e.what() << std::endl;
                    recv();
                }
            } else if (error) {
                std::cerr << "[Server] Receive error: " << error.message() << std::endl;
            }
        }));
}

bool AsioServer::poll(MessageQueue &msg)
{
    return _inComingMessages.pop(msg);
}
