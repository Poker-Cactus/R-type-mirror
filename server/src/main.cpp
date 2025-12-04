/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../../network/include/asioServer.hpp"
#include "../include/server.hpp"
#include <iostream>

int main()
{
    try {
        Server server(std::make_shared<AsioServer>(4242));
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
