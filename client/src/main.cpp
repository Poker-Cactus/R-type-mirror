/**
 * @file main.cpp
 * @brief Client application entry point
 */

#include "Game.hpp"
#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

/**
 * @brief Display command-line usage information
 * @param programName Name of the program executable
 */
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
            << std::endl;
}

/**
 * @brief Main entry point for the R-Type client
 *
 * Parses command-line arguments for server connection details,
 * initializes the game, and runs the main game loop.
 *
 * @param argc Argument count
 * @param argv Argument values
 * @return EXIT_SUCCESS on normal exit, EXIT_FAILURE on error
 */
int main(int argc, char **argv)
{
  std::string host = "127.0.0.1";
  std::string port = "4242";

  if (argc >= 2) {
    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help") {
      printUsage(argv[0]);
      return EXIT_SUCCESS;
    }
    host = arg1;
  }
  if (argc >= 3) {
    port = argv[2];
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
