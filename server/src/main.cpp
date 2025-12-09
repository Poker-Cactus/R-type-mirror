/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../../network/include/AsioServer.hpp"
#include "../include/Server.hpp"
#include <csignal>
#include <iostream>

int main()
{
    std::signal(SIGINT, Server::signalHandler);
    try {
        Server server(std::make_shared<AsioServer>(4241));
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
