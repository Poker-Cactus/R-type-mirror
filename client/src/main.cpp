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

int SDL_main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  Game game;
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
