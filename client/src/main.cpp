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

int main()
{
    Game game;
    if (!game.init()) {
        return EXIT_FAILURE;
    }

    game.run();
    return 0;
}
