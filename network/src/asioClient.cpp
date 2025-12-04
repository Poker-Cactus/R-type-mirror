/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** asioClient.cpp
*/

#include "../include/asioClient.hpp"

AsioClient::AsioClient(const std::string &host, const std::string &port)
    : _strand(asio::make_strand(_ioContext)), _socket(_ioContext), _running(true),
      _workGuard(asio::make_work_guard(_ioContext))
{
    try {
        asio::ip::udp::resolver resolver(_ioContext);
        asio::ip::udp::endpoint serverEndpoint = *resolver.resolve(asio::ip::udp::v4(), host, port).begin();

        _socket.open(asio::ip::udp::v4());
        _serverEndpoint = serverEndpoint;
        _recvThread = std::thread([this]() { _ioContext.run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        startReceive();
        send(buildMessage(MESSAGE_PING, "PING"), 0);
    } catch (const std::exception &e) {
        std::cerr << "[Client] Init error: " << e.what() << std::endl;
        _running = false;
    }
}

AsioClient::~AsioClient()
{
    _running = false;
    _ioContext.stop();
    _workGuard.reset();
    if (_recvThread.joinable())
        _recvThread.join();
}

void AsioClient::run()
{
    while (_running) {
        MessageQueue msg;
        while (getIncomingMessage(msg)) {
            std::cout << "[Client] Received: " << msg.getData() << std::endl;
            send(buildMessage(MESSAGE_PING, "PING"), 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::string AsioClient::buildMessage(uint8_t header, const std::string &data) const
{
    return std::string(1, static_cast<char>(header)) + data;
}

void AsioClient::send(const std::string &data, const uint32_t &targetEndpointId)
{
    auto buffer = std::make_shared<std::vector<uint8_t>>(PacketHandler::serialize(data));

    _socket.async_send_to(
        asio::buffer(*buffer), _serverEndpoint,
        asio::bind_executor(_strand, [buffer](const std::error_code &error, std::size_t /*bytes_transferred*/) {
            if (error) {
                std::cerr << "[Client] Send error: " << error.message() << std::endl;
            }
        }));
}

void AsioClient::startReceive()
{
    auto recvBuffer = std::make_shared<std::array<char, BUFFER_SIZE>>();

    _socket.async_receive_from(
        asio::buffer(*recvBuffer), _serverEndpoint,
        asio::bind_executor(_strand, [this, recvBuffer](const std::error_code &error, std::size_t bytes_transferred) {
            if (!error && bytes_transferred > 0) {
                try {
                    const std::string data = PacketHandler::deserialize(*recvBuffer, bytes_transferred);
                    MessageQueue messageQueue(data, _serverEndpoint);
                    _inComingMessages.push(messageQueue);

                    startReceive();
                } catch (const std::exception &e) {
                    std::cerr << "[Client] Deserialization error: " << e.what() << std::endl;
                    startReceive();
                }
            } else if (error) {
                std::cerr << "[Client] Receive error: " << error.message() << std::endl;
            }
        }));
}

bool AsioClient::getIncomingMessage(MessageQueue &msg)
{
    return _inComingMessages.pop(msg);
}
