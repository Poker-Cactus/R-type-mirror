/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client entry point
*/

#include "../../network/include/AsioClient.hpp"
#include "../include/Client.hpp"
#include <csignal>

int main(UNUSED int argc, char **argv)
{
  try {
    Client client(std::make_shared<AsioClient>("127.0.0.1", "4241"));
    client.loop();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
