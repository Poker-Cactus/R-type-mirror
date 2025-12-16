/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** RendererSDL2.cpp
*/

#include "RendererSDL2.hpp"
#include "../../interface/KeyCodes.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
constexpr float kMillisecondsPerSecond = 1000.0F;
constexpr float kGamepadAxisMax = 32767.0F;
} // anonymous namespace

// Map SDL keycodes to generic keycodes
static int mapSDLKeyToGeneric(int sdlKey)
{
  switch (sdlKey) {
  case SDLK_RETURN:
    return KeyCode::KEY_RETURN;
  case SDLK_ESCAPE:
    return KeyCode::KEY_ESCAPE;
  case SDLK_BACKSPACE:
    return KeyCode::KEY_BACKSPACE;
  case SDLK_TAB:
    return KeyCode::KEY_TAB;
  case SDLK_SPACE:
    return KeyCode::KEY_SPACE;
  case SDLK_DELETE:
    return KeyCode::KEY_DELETE;

  case SDLK_UP:
    return KeyCode::KEY_UP;
  case SDLK_DOWN:
    return KeyCode::KEY_DOWN;
  case SDLK_LEFT:
    return KeyCode::KEY_LEFT;
  case SDLK_RIGHT:
    return KeyCode::KEY_RIGHT;

  case SDLK_a:
    return KeyCode::KEY_A;
  case SDLK_b:
    return KeyCode::KEY_B;
  case SDLK_c:
    return KeyCode::KEY_C;
  case SDLK_d:
    return KeyCode::KEY_D;
  case SDLK_e:
    return KeyCode::KEY_E;
  case SDLK_f:
    return KeyCode::KEY_F;
  case SDLK_g:
    return KeyCode::KEY_G;
  case SDLK_h:
    return KeyCode::KEY_H;
  case SDLK_i:
    return KeyCode::KEY_I;
  case SDLK_j:
    return KeyCode::KEY_J;
  case SDLK_k:
    return KeyCode::KEY_K;
  case SDLK_l:
    return KeyCode::KEY_L;
  case SDLK_m:
    return KeyCode::KEY_M;
  case SDLK_n:
    return KeyCode::KEY_N;
  case SDLK_o:
    return KeyCode::KEY_O;
  case SDLK_p:
    return KeyCode::KEY_P;
  case SDLK_q:
    return KeyCode::KEY_Q;
  case SDLK_r:
    return KeyCode::KEY_R;
  case SDLK_s:
    return KeyCode::KEY_S;
  case SDLK_t:
    return KeyCode::KEY_T;
  case SDLK_u:
    return KeyCode::KEY_U;
  case SDLK_v:
    return KeyCode::KEY_V;
  case SDLK_w:
    return KeyCode::KEY_W;
  case SDLK_x:
    return KeyCode::KEY_X;
  case SDLK_y:
    return KeyCode::KEY_Y;
  case SDLK_z:
    return KeyCode::KEY_Z;

  case SDLK_0:
    return KeyCode::KEY_0;
  case SDLK_1:
    return KeyCode::KEY_1;
  case SDLK_2:
    return KeyCode::KEY_2;
  case SDLK_3:
    return KeyCode::KEY_3;
  case SDLK_4:
    return KeyCode::KEY_4;
  case SDLK_5:
    return KeyCode::KEY_5;
  case SDLK_6:
    return KeyCode::KEY_6;
  case SDLK_7:
    return KeyCode::KEY_7;
  case SDLK_8:
    return KeyCode::KEY_8;
  case SDLK_9:
    return KeyCode::KEY_9;

  case SDLK_F11:
    return KeyCode::KEY_F11;

  default:
    return KeyCode::KEY_UNKNOWN;
  }
}

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

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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
    const float targetFrameTime = 1.0F / static_cast<float>(targetFPS);
    if (deltaTime < targetFrameTime) {
      const auto delay = static_cast<Uint32>((targetFrameTime - deltaTime) * kMillisecondsPerSecond);
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

void RendererSDL2::setFullscreen(bool fullscreen)
{
  this->fullscreen = fullscreen;
  SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

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

  previousKeyStates = keyStates;

  while (SDL_PollEvent(&event) != 0) {
    if (event.type == SDL_QUIT) {
      std::cout << "[RendererSDL2] SDL_QUIT event received\n";
      return false;
    }
    if (event.type == SDL_KEYDOWN) {
      int genericKey = mapSDLKeyToGeneric(event.key.keysym.sym);
      keyStates[genericKey] = true;
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        std::cout << "[RendererSDL2] ESC pressed - closing\n";
        return false;
      }
    }
    if (event.type == SDL_KEYUP) {
      int genericKey = mapSDLKeyToGeneric(event.key.keysym.sym);
      keyStates[genericKey] = false;
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

bool RendererSDL2::isKeyJustPressed(int keycode)
{
  return keyStates[keycode] && !previousKeyStates[keycode];
}

void RendererSDL2::getMousePosition(int &outX, int &outY)
{
  outX = mouseX;
  outY = mouseY;
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
  return static_cast<float>(value) / kGamepadAxisMax;
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

void RendererSDL2::getTextureSize(void *texture, int &width, int &height)
{
  if (texture != nullptr) {
    SDL_QueryTexture(static_cast<SDL_Texture *>(texture), nullptr, nullptr, &width, &height);
  } else {
    width = 0;
    height = 0;
  }
}

void RendererSDL2::drawTexture(void *texture, int posX, int posY)
{
  if (texture == nullptr) {
    return;
  }

  auto *tex = static_cast<SDL_Texture *>(texture);
  int texWidth = 0;
  int texHeight = 0;
  SDL_QueryTexture(tex, nullptr, nullptr, &texWidth, &texHeight);

  SDL_Rect dest = {posX + cameraOffsetX, posY + cameraOffsetY, texWidth, texHeight};
  SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

void RendererSDL2::drawTextureRegion(void *texture, const Rect &src, const Rect &dst)
{
  if (texture == nullptr) {
    return;
  }

  SDL_Rect srcRect = {src.x, src.y, src.width, src.height};
  SDL_Rect dstRect = {dst.x + cameraOffsetX, dst.y + cameraOffsetY, dst.width, dst.height};
  SDL_RenderCopy(renderer, static_cast<SDL_Texture *>(texture), &srcRect, &dstRect);
}

void RendererSDL2::drawTextureEx(void *texture, const Rect &dst, double angle, bool flipX, bool flipY)
{
  if (texture == nullptr) {
    return;
  }

  SDL_Rect dest = {dst.x + cameraOffsetX, dst.y + cameraOffsetY, dst.width, dst.height};
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

void RendererSDL2::drawText(void *font, const std::string &text, int posX, int posY, const Color &color)
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
    SDL_Rect dest = {posX - cameraOffsetX, posY - cameraOffsetY, surface->w, surface->h};
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
void RendererSDL2::drawRect(int posX, int posY, int width, int height, const Color &color)
{
  SDL_Rect rect{posX + cameraOffsetX, posY + cameraOffsetY, width, height};
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(renderer, &rect);
}

void RendererSDL2::drawRectOutline(int posX, int posY, int width, int height, const Color &color)
{
  SDL_Rect rect{posX + cameraOffsetX, posY + cameraOffsetY, width, height};
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawRect(renderer, &rect);
}

void RendererSDL2::drawLine(int startX, int startY, int endX, int endY, const Color &color)
{
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawLine(renderer, startX + cameraOffsetX, startY + cameraOffsetY, endX + cameraOffsetX,
                     endY + cameraOffsetY);
}

void RendererSDL2::drawCircle(const Circle &circle, const Color &color)
{
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  int currX = circle.radius;
  int currY = 0;
  int err = 0;

  while (currX >= currY) {
    SDL_RenderDrawPoint(renderer, circle.centerX + currX + cameraOffsetX, circle.centerY + currY + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX + currY + cameraOffsetX, circle.centerY + currX + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX - currY + cameraOffsetX, circle.centerY + currX + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX - currX + cameraOffsetX, circle.centerY + currY + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX - currX + cameraOffsetX, circle.centerY - currY + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX - currY + cameraOffsetX, circle.centerY - currX + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX + currY + cameraOffsetX, circle.centerY - currX + cameraOffsetY);
    SDL_RenderDrawPoint(renderer, circle.centerX + currX + cameraOffsetX, circle.centerY - currY + cameraOffsetY);

    if (err <= 0) {
      currY += 1;
      err += 2 * currY + 1;
    }
    if (err > 0) {
      currX -= 1;
      err -= 2 * currX + 1;
    }
  }
}

void RendererSDL2::drawCircleFilled(const Circle &circle, const Color &color)
{
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  for (int col = 0; col < circle.radius * 2; ++col) {
    for (int row = 0; row < circle.radius * 2; ++row) {
      int deltaX = circle.radius - col;
      int deltaY = circle.radius - row;
      if (((deltaX * deltaX) + (deltaY * deltaY)) <= (circle.radius * circle.radius)) {
        SDL_RenderDrawPoint(renderer, circle.centerX + deltaX + cameraOffsetX, circle.centerY + deltaY + cameraOffsetY);
      }
    }
  }
}

void RendererSDL2::drawPoint(int posX, int posY, const Color &color)
{
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawPoint(renderer, posX + cameraOffsetX, posY + cameraOffsetY);
}

// === ICamera ===
void RendererSDL2::setViewport(int posX, int posY, int width, int height)
{
  SDL_Rect viewport = {posX, posY, width, height};
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

void RendererSDL2::getCameraOffset(int &outOffsetX, int &outOffsetY) const
{
  outOffsetX = cameraOffsetX;
  outOffsetY = cameraOffsetY;
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
bool RendererSDL2::checkCollisionRects(int leftX, int leftY, int leftWidth, int leftHeight, int rightX, int rightY,
                                       int rightWidth, int rightHeight)
{
  return (leftX < rightX + rightWidth && leftX + leftWidth > rightX && leftY < rightY + rightHeight &&
          leftY + leftHeight > rightY);
}

bool RendererSDL2::checkCollisionCircles(const Circle &circle1, const Circle &circle2)
{
  const int deltaX = circle2.centerX - circle1.centerX;
  const int deltaY = circle2.centerY - circle1.centerY;
  const int distanceSquared = (deltaX * deltaX) + (deltaY * deltaY);
  const int radiusSum = circle1.radius + circle2.radius;
  return distanceSquared <= (radiusSum * radiusSum);
}

bool RendererSDL2::checkPointInRect(int pointX, int pointY, int rectX, int rectY, int rectW, int rectH)
{
  return (pointX >= rectX && pointX <= rectX + rectW && pointY >= rectY && pointY <= rectY + rectH);
}
