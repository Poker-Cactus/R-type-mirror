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

//     netMsg.setMessageType("PONG");
//     netMsg.setPayload(capnp::Data::Reader(reinterpret_cast<const capnp::byte *>(message.data()), message.size()));
//     netMsg.setTimestamp(std::chrono::system_clock::now().time_since_epoch().count());

//     // Sérialiser en buffer
//     auto serialized = capnp::messageToFlatArray(messageBuilder);
//     auto bytes = serialized.asBytes();

//     // Copier dans un buffer partagé
//     auto buffer = std::make_shared<std::vector<capnp::byte>>(bytes.begin(), bytes.end());

//     _socket.async_send_to(
//         asio::buffer(*buffer), targetEndpoint,
//         asio::bind_executor(_strand, [buffer](const std::error_code &error, std::size_t /*bytes_transferred*/) {
//             if (error) {
//                 std::cerr << "Erreur d'envoi: " << error.message() << std::endl;
//             }
//         }));
// }