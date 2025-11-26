#include "RendererSDL2.hpp"
#include <stdexcept>

RendererSDL2::RendererSDL2(int width, int height)
    : windowWidth(width), windowHeight(height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    window = SDL_CreateWindow(
        "SDL Backend Window",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        throw std::runtime_error(SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        throw std::runtime_error(SDL_GetError());
    }
}

RendererSDL2::~RendererSDL2() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void RendererSDL2::clear(const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void RendererSDL2::present() {
    SDL_RenderPresent(renderer);
}

void RendererSDL2::drawRect(int x, int y, int w, int h, const Color& color) {
    SDL_Rect rect{x, y, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}
