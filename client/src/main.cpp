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
  std::cout << "Usage: " << programName << " [OPTIONS] [HOST] [PORT]\n"
            << "\n"
            << "Arguments:\n"
            << "  HOST    Server hostname or IP address (default: 127.0.0.1)\n"
            << "  PORT    Server port number (default: 4242)\n"
            << "\n"
            << "Options:\n"
            << "  -h, --help          Display this help message and exit\n"
            << "  -r RENDERER        Choose renderer module: sdl2 or sfml (default: sdl2)\n"
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
  std::string rendererType = "auto"; // auto means default based on platform

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return EXIT_SUCCESS;
    } else if (arg.substr(0, 11) == "-renderer=") {
      rendererType = arg.substr(11);
      if (rendererType != "sdl2" && rendererType != "sfml") {
        std::cerr << "Error: Invalid renderer type '" << rendererType << "'. Must be 'sdl2' or 'sfml'." << std::endl;
        return EXIT_FAILURE;
      }
    } else if (arg == "-r") {
      if (i + 1 < argc) {
        rendererType = argv[++i];
        if (rendererType != "sdl2" && rendererType != "sfml") {
          std::cerr << "Error: Invalid renderer type '" << rendererType << "'. Must be 'sdl2' or 'sfml'." << std::endl;
          return EXIT_FAILURE;
        }
      } else {
        std::cerr << "Error: -r requires a value (sdl2 or sfml)." << std::endl;
        return EXIT_FAILURE;
      }
    } else if (host == "127.0.0.1" && arg.find('.') != std::string::npos) {
      // Assume it's the host if it contains a dot and host hasn't been set yet
      host = arg;
    } else if (port == "4242" && std::all_of(arg.begin(), arg.end(), ::isdigit)) {
      // Assume it's the port if it's all digits and port hasn't been set yet
      port = arg;
    } else {
      std::cerr << "Error: Unknown argument '" << arg << "'" << std::endl;
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  // Set default renderer based on platform if auto
  if (rendererType == "auto") {
#ifdef __linux__
    rendererType = "sdl2";
#else
    rendererType = "sdl2";
#endif
  }

  Game game(host, port, rendererType);
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
