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
#include <iostream>

void LoadingMenu::init()
{
  try {
    const int fontSize = 24;
    backgroundTexture = m_renderer->loadTexture("client/assets/background/starfield.png");
    font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", fontSize);
    planet = m_renderer->loadTexture("client/assets/moon-pack/moon1.png");
    music = m_renderer->loadMusic("client/assets/audios/loadingMusic.mp3");
    if (music) {
      m_renderer->playMusic(music);
    }
  } catch (const std::exception &e) {
    backgroundTexture = nullptr;
    font = nullptr;
    planet = nullptr;
  }
}

void LoadingMenu::render(int winWidth, int winHeight,
                         MenuState *currentState)
{
  // CRITICAL: Check renderer first before any calls
  if (m_renderer == nullptr) {
    std::cerr << "[LoadingMenu] ERROR: renderer is nullptr!" << std::endl;
    return;
  }

  std::cout << "[LoadingMenu] Rendering... renderer=" << m_renderer << std::endl;

  // std::cout << "[LoadingMenu] Checking loadingScreen..." << std::endl;
  // if (loadingScreen != nullptr && loadingScreen->isActive()) {
  //   std::cout << "[LoadingMenu] LoadingScreen active, updating..." << std::endl;
  //   if (loadingScreen->update(winWidth, winHeight)) {
  //     *currentState = MenuState::MAIN_MENU;
  //   }
  //   return;
  // }

  std::cout << "[LoadingMenu] Checking isZooming..." << std::endl;
  if (isZooming) {
    std::cout << "[LoadingMenu] isZooming=true, getting deltaTime..." << std::endl;
    zoomTimer += m_renderer->getDeltaTime();
    std::cout << "[LoadingMenu] Got deltaTime" << std::endl;

    float progress = zoomTimer / zoomDuration;

    if (progress >= 1.0f) {
      isZooming = false;
      zoomTimer = 0.0f;
      zoomScale = 0.3f;
      // if (loadingScreen != nullptr) {
      //   loadingScreen->start();
      // }
      return;
    }

    zoomScale = 0.3f + (progress * progress * 9.7f);
  }

  std::cout << "[LoadingMenu] Getting deltaTime for backgroundOffsetX..." << std::endl;
  float deltaTime = m_renderer->getDeltaTime();
  std::cout << "[LoadingMenu] Got deltaTime=" << deltaTime << std::endl;
  backgroundOffsetX += deltaTime * 20.0f;
  if (backgroundOffsetX >= winWidth) {
    backgroundOffsetX = 0.0f;
  }

  if (backgroundTexture != nullptr) {
    m_renderer->drawTextureEx(backgroundTexture, static_cast<int>(backgroundOffsetX), 0, winWidth, winHeight, 0.0, false,
                            false);
    m_renderer->drawTextureEx(backgroundTexture, static_cast<int>(backgroundOffsetX - winWidth), 0, winWidth, winHeight,
                            0.0, false, false);
  }
  if (planet != nullptr) {
    int planetWidth, planetHeight;
    m_renderer->getTextureSize(planet, planetWidth, planetHeight);

    float scale = static_cast<float>(std::min(winWidth, winHeight)) * zoomScale /
      static_cast<float>(std::max(planetWidth, planetHeight));

    int scaledWidth = static_cast<int>(planetWidth * scale);
    int scaledHeight = static_cast<int>(planetHeight * scale);

    int planetX = (winWidth - scaledWidth) / 2;
    int planetY = (winHeight - scaledHeight) / 2;

    m_renderer->drawTextureEx(planet, planetX, planetY, scaledWidth, scaledHeight, 0.0, false, false);
  }
  if (font != nullptr) {
    blinkTimer += m_renderer->getDeltaTime();
    float opacity = (std::sin(blinkTimer * 3.5f) + 1.0f) / 2.0f;
    int alpha = static_cast<int>(50 + opacity * 205);

    std::string text = "Press enter to start ...";
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(font, text, textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight - textHeight) / 1.1;

    m_renderer->drawText(font, text, x, y, {255, 255, 255, static_cast<unsigned char>(alpha)});
  }
}

void LoadingMenu::process()
{
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN) && !isZooming) {
    isZooming = true;
    zoomTimer = 0.0f;
  }
}
