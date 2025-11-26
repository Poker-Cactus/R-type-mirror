#pragma once
#include "../../client/interface/IRender.hpp"
#include <SDL2/SDL.h>

class RendererSDL2 : public IRenderer {
public:
    RendererSDL2(int width = 800, int height = 600);
    ~RendererSDL2();

    void clear(const Color& color = {0,0,0,255}) override;
    void present() override;
    void drawRect(int x, int y, int w, int h, const Color& color = {255,255,255,255}) override;

    int getWindowWidth() const override { return windowWidth; }
    int getWindowHeight() const override { return windowHeight; }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int windowWidth;
    int windowHeight;
};
