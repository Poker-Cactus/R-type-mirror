/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../include/server.hpp"

int main()
{
    try {
        UdpServer server(4242);
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
