/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.hpp
*/

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#include <asio.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>

// Une structure simple pour ta SafeQueue
template<typename T>
class SafeQueue {
public:
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push_back(value);
    }

    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty()) return false;
        value = q.front();
        q.pop_front();
        return true;
    }
private:
    std::deque<T> q;
    std::mutex mtx;
};

// Ton serveur UDP
class UdpServer {
public:
    UdpServer(asio::io_context& io_context, short port)
        : socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
        start_receive();
    }

    // Méthode pour envoyer des données à un client spécifique
    void send(const std::string& message, asio::ip::udp::endpoint target_endpoint) {
        socket_.async_send_to(
            asio::buffer(message), target_endpoint,
            [](const std::error_code& error, std::size_t /*bytes_transferred*/) {
                if (error) {
                    std::cerr << "Erreur d'envoi: " << error.message() << std::endl;
                }
            });
    }

    // Accès à la queue pour le moteur de jeu
    bool get_incoming_message(std::string& msg) {
        return incoming_messages_.pop(msg);
    }

private:
    void start_receive() {
        // On prépare le buffer et on attend un paquet
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), remote_endpoint_,
            [this](const std::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    // 1. On capture le message
                    std::string message(recv_buffer_.data(), bytes_transferred);
                    
                    // 2. On le met dans la queue (Thread Safe !)
                    // Dans un vrai R-Type, tu stockerais une struct {header, body, endpoint_id}
                    incoming_messages_.push(message); 

                    // 3. On relance l'écoute immédiatement
                    start_receive();
                } else {
                    std::cerr << "Erreur de réception: " << error.message() << std::endl;
                }
            });
    }

    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_; // Stocke qui nous a envoyé le dernier paquet
    std::array<char, 1024> recv_buffer_;      // Buffer de réception
    SafeQueue<std::string> incoming_messages_;
};
