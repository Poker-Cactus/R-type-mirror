/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/server.hpp"

UdpServer::UdpServer(short port)
    : _running(true), _work_guard(asio::make_work_guard(_io_context)),
      _socket(_io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), _strand(asio::make_strand(_io_context))
{
    int nbOfThreads = std::thread::hardware_concurrency();

    for (int i = 0; i < nbOfThreads; i++) {
        _threadPool.emplace_back([this]() { _io_context.run(); });
    }
    startReceive();

    run();
}

UdpServer::~UdpServer()
{
    _work_guard.reset();
    _io_context.stop();
    for (auto &t : _threadPool) {
        if (t.joinable())
            t.join();
    }
    _socket.close();
}

void UdpServer::run()
{
    while (_running) {
        MessageQueue msg;
        while (getIncomingMessage(msg)) {
            std::cout << "Client says: " << msg.getData() << std::endl;
            asyncSend("PONG", msg.getSenderEndpoint());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void UdpServer::asyncSend(const std::string &message, asio::ip::udp::endpoint target_endpoint)
{
    // Créer un message Cap'n Proto
    capnp::MallocMessageBuilder messageBuilder;
    auto netMsg = messageBuilder.initRoot<NetworkMessage>();

    netMsg.setMessageType("PONG");
    netMsg.setPayload(capnp::Data::Reader(reinterpret_cast<const capnp::byte *>(message.data()), message.size()));
    netMsg.setTimestamp(std::chrono::system_clock::now().time_since_epoch().count());

    // Sérialiser en buffer
    auto serialized = capnp::messageToFlatArray(messageBuilder);
    auto bytes = serialized.asBytes();

    // Copier dans un buffer partagé
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
                    // Désérialiser avec Cap'n Proto
                    capnp::FlatArrayMessageReader reader(
                        kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word *>(_recv_buffer.data()),
                                                        bytes_transferred / sizeof(capnp::word)));

                    auto netMsg = reader.getRoot<NetworkMessage>();
                    std::string messageType = netMsg.getMessageType();
                    auto payload = netMsg.getPayload();

                    std::string message(reinterpret_cast<const char *>(payload.begin()), payload.size());

                    MessageQueue messageQueue(message, _remote_endpoint);
                    _inComingMessages.push(messageQueue);

                    startReceive();
                } catch (const std::exception &e) {
                    std::cerr << "Erreur de désérialisation: " << e.what() << std::endl;
                }
            } else {
                std::cerr << "Erreur de réception: " << error.message() << std::endl;
            }
        }));
}
