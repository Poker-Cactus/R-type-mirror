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

static void printUsage(const char *programName)
{
  std::cout << "Usage: " << programName << " [HOST] [PORT]\n"
            << "\n"
            << "Arguments:\n"
            << "  HOST    Server hostname or IP address (default: 127.0.0.1)\n"
            << "  PORT    Server port number (default: 4242)\n"
            << "\n"
            << "Options:\n"
            << "  -h, --help    Display this help message and exit\n"
            << "\n"
            << "Examples:\n"
            << "  " << programName << "                      # Connect to 127.0.0.1:4242\n"
            << "  " << programName << " 192.168.1.100        # Connect to 192.168.1.100:4242\n"
            << "  " << programName << " 192.168.1.100 8080   # Connect to 192.168.1.100:8080\n"
            << "  " << programName << " game.server.com 4242 # Connect to game.server.com:4242\n"
            << std::endl;
}

int main(int argc, char **argv)
{
  std::string host = "127.0.0.1";
  std::string port = "4242";

  // Check for help flag
  if (argc >= 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
    printUsage(argv[0]);
    return EXIT_SUCCESS;
  }

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
