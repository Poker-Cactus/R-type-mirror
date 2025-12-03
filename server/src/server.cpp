/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/server.hpp"

Server::Server() : _networkManager(std::make_unique<NetworkManager>(4242)) {}

Server::Server(const int port) : _networkManager(std::make_unique<NetworkManager>(port)) {}

Server::~Server() {}

// void UdpServer::asyncSend(const std::string &message, asio::ip::udp::endpoint targetEndpoint)
// {
//     // Créer un message Cap'n Proto
//     capnp::MallocMessageBuilder messageBuilder;
//     auto netMsg = messageBuilder.initRoot<NetworkMessage>();

void UdpServer::run()
{
    while (_running) {
        MessageQueue msg;
        while (getIncomingMessage(msg)) {
            std::cout << "Message complet: '" << msg.getFullMessage() << "'" << std::endl;
            asyncSend("PONGGGG", msg.getSenderEndpoint());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void UdpServer::asyncSend(const std::string &message, asio::ip::udp::endpoint target_endpoint)
{
    capnp::MallocMessageBuilder messageBuilder;
    auto netMsg = messageBuilder.initRoot<NetworkMessage>();

    netMsg.setMessageType(message);

    auto serialized = capnp::messageToFlatArray(messageBuilder);
    auto bytes = serialized.asBytes();

    auto buffer = std::make_shared<std::vector<capnp::byte>>(bytes.begin(), bytes.end());

    _socket.async_send_to(
        asio::buffer(*buffer), target_endpoint,
        asio::bind_executor(_strand, [buffer](const std::error_code &error, std::size_t /*bytes_transferred*/) {
            if (error) {
                std::cerr << "Erreur d'envoi: " << error.message() << std::endl;
            }
        }));
}

bool UdpServer::getIncomingMessage(MessageQueue &msg)
{
    return _inComingMessages.pop(msg);
}

void UdpServer::startReceive()
{
    _socket.async_receive_from(
        asio::buffer(_recv_buffer), _remote_endpoint,
        asio::bind_executor(_strand, [this](const std::error_code &error, std::size_t bytes_transferred) {
            if (!error) {
                try {
                    size_t word_count = (bytes_transferred + sizeof(capnp::word) - 1) / sizeof(capnp::word);

                    kj::Array<capnp::word> aligned_buffer = kj::heapArray<capnp::word>(word_count);
                    memcpy(aligned_buffer.begin(), _recv_buffer.data(), bytes_transferred);

                    capnp::FlatArrayMessageReader reader(aligned_buffer.asPtr());

                    auto netMsg = reader.getRoot<NetworkMessage>();
                    std::string messageType = netMsg.getMessageType().cStr();

                    MessageQueue messageQueue(messageType, _remote_endpoint);
                    _inComingMessages.push(messageQueue);

                    startReceive();
                } catch (const std::exception &e) {
                    std::cerr << "Erreur de désérialisation: " << e.what() << std::endl;
                    startReceive();
                }
            } else {
                std::cerr << "Erreur de réception: " << error.message() << std::endl;
            }
        }));
}
