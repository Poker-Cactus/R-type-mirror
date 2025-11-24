/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "network/main.hpp"

int main() {
    try {
        asio::io_context io_context;
        
        // Création du serveur sur le port 4242
        UdpServer server(io_context, 4242);

        // Le thread réseau tourne en arrière-plan
        std::thread network_thread([&io_context](){
            io_context.run(); 
        });

        // Boucle de jeu (Game Loop)
        while (true) {
            // 1. Lire les inputs réseau
            std::string msg;
            while (server.get_incoming_message(msg)) {
                std::cout << "Moteur de jeu a reçu : " << msg << std::endl;
                // Ici : parser le message et mettre à jour l'ECS
            }

            // 2. Update Logic (ECS systems)
            
            // 3. Render / Send updates to clients
            // server.send("Position Update...", client_endpoint);

            // Simulation de frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); 
        }

        network_thread.join();

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
