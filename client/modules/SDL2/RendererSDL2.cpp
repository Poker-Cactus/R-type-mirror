#include "RendererSDL2.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <cmath>
#include <stdexcept>
#include <string>

RendererSDL2::RendererSDL2(int width, int height) : windowWidth(width), windowHeight(height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    if (TTF_Init() != 0) {
        SDL_Quit();
        throw std::runtime_error(TTF_GetError());
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        TTF_Quit();
        SDL_Quit();
        throw std::runtime_error(IMG_GetError());
    }

    window = SDL_CreateWindow("SDL Backend Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        IMG_Quit();
        TTF_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        IMG_Quit();
        TTF_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i) == SDL_TRUE) {
            SDL_GameController *pad = SDL_GameControllerOpen(i);
            if (pad != nullptr) {
                gamepads.push_back(pad);
            }
        }
    }

    lastFrameTime = SDL_GetPerformanceCounter();
}

RendererSDL2::~RendererSDL2()
{
    for (auto *pad : gamepads) {
        SDL_GameControllerClose(pad);
    }
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
    }
    if (window != nullptr) {
        SDL_DestroyWindow(window);
    }
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void RendererSDL2::clear(const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void RendererSDL2::present()
{
    SDL_RenderPresent(renderer);

    Uint64 now = SDL_GetPerformanceCounter();
    deltaTime = static_cast<float>(now - lastFrameTime) / static_cast<float>(SDL_GetPerformanceFrequency());
    lastFrameTime = now;

    if (deltaTime > 0.0F) {
        currentFPS = static_cast<int>(1.0F / deltaTime);
    }

    if (targetFPS > 0) {
        float targetFrameTime = 1.0F / static_cast<float>(targetFPS);
        if (deltaTime < targetFrameTime) {
            Uint32 delay = static_cast<Uint32>((targetFrameTime - deltaTime) * 1000.0F);
            SDL_Delay(delay);
        }
    }
}

void RendererSDL2::setWindowSize(int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    SDL_SetWindowSize(window, width, height);
}

void RendererSDL2::setWindowTitle(const std::string &title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

void RendererSDL2::setFullscreen(bool fs)
{
    fullscreen = fs;
    SDL_SetWindowFullscreen(window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

    // Mettre à jour les dimensions de la fenêtre après le changement
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
}

bool RendererSDL2::isFullscreen() const
{
    return fullscreen;
}

bool RendererSDL2::pollEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            keyStates[event.key.keysym.sym] = true;
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                return false;
            }
        }
        if (event.type == SDL_KEYUP) {
            keyStates[event.key.keysym.sym] = false;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouseButtonStates[event.button.button] = true;
        }
        if (event.type == SDL_MOUSEBUTTONUP) {
            mouseButtonStates[event.button.button] = false;
        }
        if (event.type == SDL_MOUSEMOTION) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;
        }
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            }
        }
    }
    return true;
}

bool RendererSDL2::isKeyPressed(int keycode)
{
    return keyStates[keycode];
}

bool RendererSDL2::isKeyReleased(int keycode)
{
    return !keyStates[keycode];
}

void RendererSDL2::getMousePosition(int &x, int &y)
{
    x = mouseX;
    y = mouseY;
}

bool RendererSDL2::isMouseButtonPressed(int button)
{
    return mouseButtonStates[button];
}

int RendererSDL2::getNumGamepads()
{
    return static_cast<int>(gamepads.size());
}

bool RendererSDL2::isGamepadButtonPressed(int gamepadIndex, int button)
{
    if (gamepadIndex < 0 || gamepadIndex >= static_cast<int>(gamepads.size())) {
        return false;
    }
    return SDL_GameControllerGetButton(gamepads[gamepadIndex], static_cast<SDL_GameControllerButton>(button)) != 0;
}

float RendererSDL2::getGamepadAxis(int gamepadIndex, int axis)
{
    if (gamepadIndex < 0 || gamepadIndex >= static_cast<int>(gamepads.size())) {
        return 0.0F;
    }
    Sint16 value = SDL_GameControllerGetAxis(gamepads[gamepadIndex], static_cast<SDL_GameControllerAxis>(axis));
    return static_cast<float>(value) / 32767.0F;
}

// === ITexture ===
void *RendererSDL2::loadTexture(const std::string &filepath)
{
    SDL_Surface *surface = IMG_Load(filepath.c_str());
    if (surface == nullptr) {
        throw std::runtime_error("Failed to load texture: " + filepath + " - " + IMG_GetError());
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        throw std::runtime_error("Failed to create texture from surface");
    }

    // Activer le filtrage linéaire pour un meilleur scaling
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);

    return texture;
}

void RendererSDL2::freeTexture(void *texture)
{
    if (texture != nullptr) {
        SDL_DestroyTexture(static_cast<SDL_Texture *>(texture));
    }
}

void RendererSDL2::drawTexture(void *texture, int x, int y)
{
    if (texture == nullptr) {
        return;
    }

    SDL_Texture *tex = static_cast<SDL_Texture *>(texture);
    int w = 0;
    int h = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);

    SDL_Rect dest = {x + cameraOffsetX, y + cameraOffsetY, w, h};
    SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

void RendererSDL2::drawTextureRegion(void *texture, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY,
                                     int dstW, int dstH)
{
    if (texture == nullptr) {
        return;
    }

    SDL_Rect src = {srcX, srcY, srcW, srcH};
    SDL_Rect dest = {dstX + cameraOffsetX, dstY + cameraOffsetY, dstW, dstH};
    SDL_RenderCopy(renderer, static_cast<SDL_Texture *>(texture), &src, &dest);
}

void RendererSDL2::drawTextureEx(void *texture, int x, int y, int w, int h, double angle, bool flipX, bool flipY)
{
    if (texture == nullptr) {
        return;
    }

    SDL_Rect dest = {x + cameraOffsetX, y + cameraOffsetY, w, h};
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flipX) {
        flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    }
    if (flipY) {
        flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
    }

    SDL_RenderCopyEx(renderer, static_cast<SDL_Texture *>(texture), nullptr, &dest, angle, nullptr, flip);
}

// === IText ===
void *RendererSDL2::loadFont(const std::string &filepath, int fontSize)
{
    TTF_Font *font = TTF_OpenFont(filepath.c_str(), fontSize);
    if (font == nullptr) {
        throw std::runtime_error("Failed to load font: " + filepath + " - " + TTF_GetError());
    }
    return font;
}

void RendererSDL2::freeFont(void *font)
{
    if (font != nullptr) {
        TTF_CloseFont(static_cast<TTF_Font *>(font));
    }
}

void RendererSDL2::drawText(void *font, const std::string &text, int x, int y, const Color &color)
{
    if (font == nullptr || text.empty()) {
        return;
    }

    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface *surface = TTF_RenderText_Blended(static_cast<TTF_Font *>(font), text.c_str(), sdlColor);
    if (surface == nullptr) {
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture != nullptr) {
        SDL_Rect dest = {x - cameraOffsetX, y - cameraOffsetY, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void RendererSDL2::getTextSize(void *font, const std::string &text, int &width, int &height)
{
    if (font == nullptr) {
        width = 0;
        height = 0;
        return;
    }

    TTF_SizeText(static_cast<TTF_Font *>(font), text.c_str(), &width, &height);
}

// === IAudio === (Stubs - requires SDL_mixer)
void *RendererSDL2::loadSound(const std::string & /*filepath*/)
{
    return nullptr;
}

void *RendererSDL2::loadMusic(const std::string & /*filepath*/)
{
    return nullptr;
}

void RendererSDL2::playSound(void * /*sound*/, int /*loops*/)
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::playMusic(void * /*music*/, int /*loops*/)
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::pauseMusic()
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::resumeMusic()
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::stopMusic()
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::setSoundVolume(int /*volume*/)
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::setMusicVolume(int /*volume*/)
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::freeSound(void * /*sound*/)
{
    // TODO: Implement with SDL_mixer
}

void RendererSDL2::freeMusic(void * /*music*/)
{
    // TODO: Implement with SDL_mixer
}

// === IShape ===
void RendererSDL2::drawRect(int x, int y, int w, int h, const Color &color)
{
    SDL_Rect rect{x + cameraOffsetX, y + cameraOffsetY, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void RendererSDL2::drawRectOutline(int x, int y, int w, int h, const Color &color)
{
    SDL_Rect rect{x + cameraOffsetX, y + cameraOffsetY, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

void RendererSDL2::drawLine(int x1, int y1, int x2, int y2, const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, x1 + cameraOffsetX, y1 + cameraOffsetY, x2 + cameraOffsetX, y2 + cameraOffsetY);
}

void RendererSDL2::drawCircle(int centerX, int centerY, int radius, const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        SDL_RenderDrawPoint(renderer, centerX + x + cameraOffsetX, centerY + y + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX + y + cameraOffsetX, centerY + x + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX - y + cameraOffsetX, centerY + x + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX - x + cameraOffsetX, centerY + y + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX - x + cameraOffsetX, centerY - y + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX - y + cameraOffsetX, centerY - x + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX + y + cameraOffsetX, centerY - x + cameraOffsetY);
        SDL_RenderDrawPoint(renderer, centerX + x + cameraOffsetX, centerY - y + cameraOffsetY);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void RendererSDL2::drawCircleFilled(int centerX, int centerY, int radius, const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx + cameraOffsetX, centerY + dy + cameraOffsetY);
            }
        }
    }
}

void RendererSDL2::drawPoint(int x, int y, const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, x + cameraOffsetX, y + cameraOffsetY);
}

// === ICamera ===
void RendererSDL2::setViewport(int x, int y, int w, int h)
{
    SDL_Rect viewport = {x, y, w, h};
    SDL_RenderSetViewport(renderer, &viewport);
}

void RendererSDL2::resetViewport()
{
    SDL_RenderSetViewport(renderer, nullptr);
}

void RendererSDL2::setCameraOffset(int offsetX, int offsetY)
{
    cameraOffsetX = offsetX;
    cameraOffsetY = offsetY;
}

void RendererSDL2::getCameraOffset(int &offsetX, int &offsetY) const
{
    offsetX = cameraOffsetX;
    offsetY = cameraOffsetY;
}

// === ITime ===
float RendererSDL2::getDeltaTime() const
{
    return deltaTime;
}

int RendererSDL2::getFPS() const
{
    return currentFPS;
}

void RendererSDL2::setTargetFPS(int fps)
{
    targetFPS = fps;
}

void RendererSDL2::setVSync(bool /*enabled*/)
{
    // Note: VSync is set at renderer creation time
    // Would need to recreate renderer to change
}

// === ICollision ===
bool RendererSDL2::checkCollisionRects(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

bool RendererSDL2::checkCollisionCircles(int x1, int y1, int r1, int x2, int y2, int r2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int distanceSquared = dx * dx + dy * dy;
    int radiusSum = r1 + r2;
    return distanceSquared <= (radiusSum * radiusSum);
}

bool RendererSDL2::checkPointInRect(int px, int py, int rx, int ry, int rw, int rh)
{
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}
