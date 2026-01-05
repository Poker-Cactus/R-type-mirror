/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client entry point
*/

#include "Game.hpp"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
  std::string host = "127.0.0.1";
  std::string port = "4242";

  // Parse command line arguments
  if (argc >= 2) {
    host = argv[1];
  }
  if (argc >= 3) {
    port = argv[2];
  }

  if (argc > 1) {
    std::cout << "[Client] Connecting to " << host << ":" << port << std::endl;
  }

  Game game(host, port);
  if (!game.init()) {
    return EXIT_FAILURE;
  }

  game.run();
  return 0;
}
// Network commented out, il faudra qu'on merge vraiment les deux (juste avoir les deux trucs dans le main quoi), c'est
// juste pour la PR #include "../../network/include/AsioClient.hpp"
// #include "../include/Client.hpp"
// #include <csignal>

// int main(UNUSED int argc, char **argv)
//{
//  try {
//    Client client(std::make_shared<AsioClient>("127.0.0.1", "4241"));
//    client.loop();
//  } catch (std::exception &e) {
//    std::cerr << e.what() << std::endl;
//  }
//  return 0;
//}
