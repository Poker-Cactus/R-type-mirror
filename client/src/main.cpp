/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client entry point
*/

#include "../../network/include/AsioClient.hpp"
#include "../include/Client.hpp"
#include <atomic>
#include <csignal>

std::atomic<bool> g_running{true};

void signalHandler(int signum)
{
    (void)signum;
    g_running = false;
}

int main(UNUSED int argc, char **argv)
{
    std::signal(SIGINT, signalHandler);
    try {
        Client client(std::make_shared<AsioClient>("127.0.0.1", "4241"));
        client.loop(g_running);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
