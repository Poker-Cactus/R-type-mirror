/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client entry point
*/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "../interface/IRender.hpp"
#include "../ModuleLoader.hpp"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// int main_loop(SDL_Renderer *renderer) {
//     while (true) {
//         SDL_SetRenderDrawColor(renderer, 255,255,255,250);
//         SDL_RenderClear(renderer);
//         SDL_RenderPresent(renderer);
//     }
// }

// int main()
// {
//     SDL_Window *window = SDL_CreateWindow("Example: 0", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
//     SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
//     main_loop(renderer);
//     SDL_DestroyWindow(window);
//     SDL_DestroyRenderer(renderer);
//     SDL_Quit();
//     return EXIT_SUCCESS;
// }

int main() {
    Module<IRenderer> mod(
    "./modules/SDLBackend/SDLBackend.so",  // ou .dll sous Windows
    "createRenderer",
    "destroyRenderer"
    );

    IRenderer* renderer = mod.create();
    renderer->clear();
    renderer->present();
    mod.destroy(renderer);
}