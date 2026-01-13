/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** LoadingMenu.cpp
*/

#include "LoadingMenu.hpp"
#include "../../../interface/Geometry.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <cmath>

void LoadingMenu::init(IRenderer *renderer)
{
  try {
    const int fontSize = 24;
    backgroundTexture = renderer->loadTexture("client/assets/background/starfield.png");
    font = renderer->loadFont("client/assets/font.opf/r-type.otf", fontSize);
    planet = renderer->loadTexture("client/assets/moon-pack/moon1.png");
    music = renderer->loadMusic("client/assets/audios/loadingMusic.mp3");
    if (music) {
      renderer->playMusic(music);
    }
  } catch (const std::exception &e) {
    backgroundTexture = nullptr;
    font = nullptr;
    planet = nullptr;
  }
}

void LoadingMenu::render(int winWidth, int winHeight, IRenderer *renderer, LoadingScreen *loadingScreen,
                         MenuState *currentState)
{
  if (loadingScreen != nullptr && loadingScreen->isActive()) {
    if (loadingScreen->update(winWidth, winHeight)) {
      *currentState = MenuState::MAIN_MENU;
    }
    return;
  }

  if (isZooming) {
    zoomTimer += renderer->getDeltaTime();

    float progress = zoomTimer / zoomDuration;

    if (progress >= 1.0f) {
      isZooming = false;
      zoomTimer = 0.0f;
      zoomScale = 0.3f;
      if (loadingScreen != nullptr) {
        loadingScreen->start();
      }
      return;
    }

    zoomScale = 0.3f + (progress * progress * 9.7f);
  }

  backgroundOffsetX += renderer->getDeltaTime() * 20.0f;
  if (backgroundOffsetX >= winWidth) {
    backgroundOffsetX = 0.0f;
  }

  if (backgroundTexture != nullptr) {
    renderer->drawTextureEx(backgroundTexture, static_cast<int>(backgroundOffsetX), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(backgroundTexture, static_cast<int>(backgroundOffsetX - winWidth), 0, winWidth, winHeight,
                            0.0, false, false);
  }
  if (planet != nullptr) {
    int planetWidth, planetHeight;
    renderer->getTextureSize(planet, planetWidth, planetHeight);

    float scale = static_cast<float>(std::min(winWidth, winHeight)) * zoomScale /
      static_cast<float>(std::max(planetWidth, planetHeight));

    int scaledWidth = static_cast<int>(planetWidth * scale);
    int scaledHeight = static_cast<int>(planetHeight * scale);

    int planetX = (winWidth - scaledWidth) / 2;
    int planetY = (winHeight - scaledHeight) / 2;

    renderer->drawTextureEx(planet, planetX, planetY, scaledWidth, scaledHeight, 0.0, false, false);
  }
  if (font != nullptr) {
    blinkTimer += renderer->getDeltaTime();
    float opacity = (std::sin(blinkTimer * 3.5f) + 1.0f) / 2.0f;
    int alpha = static_cast<int>(50 + opacity * 205);

    std::string text = "Press enter to start ...";
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(font, text, textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight - textHeight) / 1.1;

    renderer->drawText(font, text, x, y, {255, 255, 255, static_cast<unsigned char>(alpha)});
  }
}

void LoadingMenu::process(IRenderer *renderer)
{
  if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN) && !isZooming) {
    isZooming = true;
    zoomTimer = 0.0f;
  }
}
