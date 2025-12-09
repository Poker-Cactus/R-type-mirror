/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../../network/include/AsioServer.hpp"
#include "../include/Server.hpp"
#include <atomic>
#include <csignal>
#include <iostream>

std::atomic<bool> g_running{true};

void signalHandler(int signum)
{
    (void)signum;
    g_running = false;
}

int main()
{
    std::signal(SIGINT, signalHandler);
    try {
        Server server(std::make_shared<AsioServer>(4241));
        server.run(g_running);
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
