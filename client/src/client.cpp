/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** client.cpp
*/

#include "../include/client.hpp"

UdpClient::UdpClient(const std::string &host, const std::string &port)
    : _running(true), _socket(_io_context), _strand(asio::make_strand(_io_context))
{
    asio::ip::udp::resolver resolver(_io_context);
    asio::ip::udp::endpoint server_endpoint = *resolver.resolve(asio::ip::udp::v4(), host, port).begin();

    _socket.open(asio::ip::udp::v4());
    _socket.connect(server_endpoint);

    startReceive();

    _network_thread = std::thread([this]() { _io_context.run(); });
}

UdpClient::~UdpClient()
{
    _running = false;
    _io_context.stop();
    if (_network_thread.joinable())
        _network_thread.join();
    _socket.close();
}

void UdpClient::send(const std::string &msg)
{
    _socket.async_send(
        asio::buffer(msg),
        asio::bind_executor(_strand, [](const std::error_code &error, std::size_t /*bytes_transferred*/) {
            if (error) {
                std::cerr << "Erreur d'envoi : " << error.message() << std::endl;
            }
        }));
}

bool UdpClient::getIncomingMessage(std::string &msg)
{
    return _inComingMessages.pop(msg);
}

void UdpClient::recvLoop()
{
    std::string serverMsg;

    while (getIncomingMessage(serverMsg)) {
        std::cout << "[SERVEUR] " << serverMsg << std::endl;
        // Ici : update de la position des entités ennemies
    }
}

void UdpClient::loop()
{
    while (_running) {
        // --- 1. Gestion des Inputs (Input System) ---
        // Exemple : Si j'appuie sur ESPACE, je tire
        // if (IsKeyPressed(KEY_SPACE)) {
        //     client.send("ACTION:SHOOT");
        // }

        // Pour l'exemple, on envoie un ping manuel :
        send(std::string(1, MESSAGE_PING) + " " + "Hello from Client!");

        // --- 2. Réseau (Network System) ---
        // On traite TOUS les paquets reçus depuis la dernière frame
        recvLoop();

        // --- 3. Rendu (Render System) ---
        // Window.clear();
        // DrawEntities();
        // Window.display();

        // Simulation 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void UdpClient::startReceive()
{
    _socket.async_receive(
        asio::buffer(_recv_buffer),
        asio::bind_executor(_strand, [this](const std::error_code &error, std::size_t bytes_transferred) {
            if (!error) {
                std::string msg(_recv_buffer.data(), bytes_transferred);
                _inComingMessages.push(msg);
                startReceive();
            } else {
                std::cerr << "Erreur de réception : " << error.message() << std::endl;
            }
        }));
}
