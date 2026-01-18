/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Menu.cpp
*/

#include "Menu.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

Menu::~Menu() {

};

bool Menu::init()
{
  try {
    std::cout << "[Menu] Loading message.png..." << std::endl;
    menuInterface = m_renderer->loadTexture("flappyBird/client/assets/sprites/message.png");
    std::cout << "[Menu] menuInterface ptr: " << menuInterface << std::endl;

    std::cout << "[Menu] Loading background-day.png..." << std::endl;
    background = m_renderer->loadTexture("flappyBird/client/assets/sprites/background-day.png");
    std::cout << "[Menu] background ptr: " << background << std::endl;
    font = m_renderer->loadFont("flappyBird/client/assets/font.opf/r-type.otf", 32);

    std::cout << "[Menu] Textures loaded successfully!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[Menu] ERROR loading textures: " << e.what() << std::endl;
  }
  return true;
};

void Menu::update(float deltaTime)
{
  // Update parallax scrolling
  parallaxOffset -= 50.0f * deltaTime; // Scroll left at 50 pixels/second

  // Update blink timer
  blinkTimer += deltaTime * blinkSpeed;
}

void Menu::render()
{
  int winWidth = m_renderer->getWindowWidth();
  int winHeight = m_renderer->getWindowHeight();

  if (background == nullptr) {
    return;
  }

  // Draw background with parallax scrolling (less zoomed)
  int bgWidth, bgHeight;
  m_renderer->getTextureSize(background, bgWidth, bgHeight);

  // Scale to 80% to dezoom a bit
  float scale = 0.8f;
  int scaledWidth = static_cast<int>(bgWidth * scale);
  int scaledHeight = static_cast<int>(bgHeight * scale);

  // Calculate parallax offset (wrap around properly for negative values)
  float wrappedOffset = std::fmod(parallaxOffset, static_cast<float>(scaledWidth));
  if (wrappedOffset > 0)
    wrappedOffset -= scaledWidth;
  int offsetX = static_cast<int>(wrappedOffset);

  // Calculate how many copies we need to fill the screen width
  int numCopies = (winWidth / scaledWidth) + 3; // +3 to ensure full coverage

  // Draw enough copies to cover entire screen width during scrolling
  for (int i = -1; i < numCopies; i++) {
    m_renderer->drawTextureEx(background, offsetX + (i * scaledWidth), (winHeight - scaledHeight) / 2, scaledWidth,
                              scaledHeight, 0.0, false, false);
  }

  if (menuInterface == nullptr) {
    std::cout << "interface DE MERDE" << std::endl;
    return;
  }

  // Draw menu interface centered, using 60% of screen height
  int msgWidth, msgHeight;
  m_renderer->getTextureSize(menuInterface, msgWidth, msgHeight);

  // Base scale (60% of screen height)
  float baseScale = 0.6f;
  float targetHeight = winHeight * baseScale;
  float msgScale = targetHeight / msgHeight;

  int finalMsgWidth = static_cast<int>(msgWidth * msgScale);
  int finalMsgHeight = static_cast<int>(msgHeight * msgScale);

  int centerX = (winWidth - finalMsgWidth) / 2;
  int centerY = (winHeight - finalMsgHeight) / 2;

  m_renderer->drawTextureEx(menuInterface, centerX, centerY, finalMsgWidth, finalMsgHeight, 0.0, false, false);

  // Draw text below the interface
  if (font != nullptr) {
    std::string text = "Press ENTER to start!";
    int textWidth, textHeight;
    m_renderer->getTextSize(font, text, textWidth, textHeight);

    int textX = (winWidth - textWidth) / 2;
    int textY = ((winHeight / 1.1));

    m_renderer->drawText(font, text, textX, textY, {255, 255, 255});
  }
}