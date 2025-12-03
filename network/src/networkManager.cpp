/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** server.cpp
*/

#include "../include/networkManager.hpp"

NetworkManager::NetworkManager(short port)
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

NetworkManager::~NetworkManager()
{
    _work_guard.reset();
    _io_context.stop();
    for (auto &t : _threadPool) {
        if (t.joinable())
            t.join();
    }
    _socket.close();
}

void NetworkManager::run()
{
    while (_running) {
        MessageQueue msg;
        while (getIncomingMessage(msg)) {
            std::cout << "Client says: " << msg.getData() << std::endl;
            send("PONG", msg.getSenderEndpoint());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

template <typename T> void NetworkManager::send(const T &data, asio::ip::udp::endpoint targetEndpoint)
{
    kj::ArrayPtr<const kj::byte> bytes;

    switch (data) {
    case std::is_same_v<T, std::string>:
        bytes = serialize(data);
        break;
    case std::is_same_v<T, kj::Array<capnp::word>>:
        bytes = data.asBytes();
        break;
    default:
        bytes = data.asBytes();
        break;
    }

    auto buffer = std::make_shared<std::vector<uint8_t>>(bytes.begin(), bytes.end());

    _socket.async_send_to(
        asio::buffer(*buffer), targetEndpoint,
        asio::bind_executor(_strand, [buffer](const std::error_code &error, std::size_t /*bytes_transferred*/) {
            if (error) {
                std::cerr << "Erreur d'envoi: " << error.message() << std::endl;
            }
        }));
}

void NetworkManager::serialize(UNUSED const std::string &data)
{
    // Créer un message Cap'n Proto
    // Membre de la classe NetworkManager
    capnp::word scratch_space[1024 * 64]; // Buffer statique de 512KB (par ex)
    // Dans la boucle
    capnp::MallocMessageBuilder messageBuilder(kj::arrayPtr(scratch_space, sizeof(scratch_space)));
    auto netMsg = messageBuilder.initRoot<NetworkMessage>();

    netMsg.setMessageType("PONG");
    netMsg.setPayload(capnp::Data::Reader(reinterpret_cast<const capnp::byte *>(message.data()), message.size()));
    netMsg.setTimestamp(std::chrono::system_clock::now().time_since_epoch().count());

    auto serialized = capnp::messageToFlatArray(messageBuilder);
    return serialized.asBytes();
}

bool NetworkManager::getIncomingMessage(MessageQueue &msg)
{
    return _inComingMessages.pop(msg);
}

std::string &NetworkManager::deserialize(const std::array<char, 1024> &recvBuffer,
                                         const std::size_t bytes_transferred) const
{
    capnp::FlatArrayMessageReader reader(kj::ArrayPtr<const capnp::word>(
        reinterpret_cast<const capnp::word *>(data.data()), bytes_transferred / sizeof(capnp::word)));

    auto netMsg = reader.getRoot<NetworkMessage>();
    std::string messageType = netMsg.getMessageType();
    auto payload = netMsg.getPayload();

    std::string message(reinterpret_cast<const char *>(payload.begin()), payload.size());
    return message;
}

void NetworkManager::startReceive()
{
    std::array<char, 1024> recvBuffer;

    _socket.async_receive_from(
        asio::buffer(recvBuffer), _remote_endpoint,
        asio::bind_executor(_strand, [this](const std::error_code &error, std::size_t bytes_transferred) {
            if (!error) {
                try {
                    const std::string data = deserialize(recvBuffer, bytes_transferred);
                    MessageQueue messageQueue(data, _remote_endpoint);
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
