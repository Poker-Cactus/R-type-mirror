/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client entry point
*/

#include "../../network/include/asioClient.hpp"
#include "../include/client.hpp"

int main(UNUSED int argc, char **argv)
{
    try {
        Client client(std::make_shared<AsioClient>("127.0.0.1", "4242"));
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
