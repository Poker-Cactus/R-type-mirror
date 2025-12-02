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
    _socket.async_send_to(
        asio::buffer(message), target_endpoint,
        asio::bind_executor(_strand, [](const std::error_code &error, std::size_t /*bytes_transferred*/) {
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
                std::string message(_recv_buffer.data(), bytes_transferred);

                // 2. On le met dans la queue (Thread Safe !)
                // Dans un vrai R-Type, tu stockerais une struct {header, body, endpoint_id}
                MessageQueue messageQueue(message, _remote_endpoint);
                _inComingMessages.push(messageQueue);

                // 3. On relance l'écoute immédiatement
                startReceive();
            } else {
                std::cerr << "Erreur de réception: " << error.message() << std::endl;
            }
        }));
}
