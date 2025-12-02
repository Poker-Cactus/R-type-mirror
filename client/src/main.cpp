/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client entry point
*/

#include "../../common/include/network/messageQueue.hpp"
#include "../include/client.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(UNUSED int argc, char **argv)
{
    try {
        UdpClient client("127.0.0.1", "4242");
        client.loop();

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
