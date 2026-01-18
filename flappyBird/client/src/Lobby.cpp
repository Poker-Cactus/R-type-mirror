/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Lobby.cpp
*/

#include "Lobby.hpp"
#include <cmath>
#include <iostream>

Lobby::~Lobby()
{
  // IRenderer doesn't have unload methods, resources are managed automatically
}

bool Lobby::init()
{
  std::cout << "[Lobby] Initializing lobby..." << std::endl;

  // Load background (same as menu for consistency)
  background = m_renderer->loadTexture("flappyBird/client/assets/sprites/background-day.png");
  if (!background) {
    std::cerr << "[Lobby] ERROR: Failed to load background texture" << std::endl;
    return false;
  }

  // Load font
  font = m_renderer->loadFont("flappyBird/client/assets/font.opf/r-type.otf", 36);
  if (!font) {
    std::cerr << "[Lobby] ERROR: Failed to load font" << std::endl;
    return false;
  }

  std::cout << "[Lobby] Lobby initialized successfully" << std::endl;
  return true;
}

void Lobby::update(float deltaTime)
{
  // Parallax scrolling
  parallaxOffset -= 50.0f * deltaTime;
}

void Lobby::render()
{
  if (!m_renderer) {
    return;
  }

  // Get window dimensions
  int windowWidth = m_renderer->getWindowWidth();
  int windowHeight = m_renderer->getWindowHeight();

  // Draw scrolling background (same scale as Menu: 80%)
  int bgWidth, bgHeight;
  m_renderer->getTextureSize(background, bgWidth, bgHeight);

  float scale = 0.8f;
  int scaledWidth = static_cast<int>(bgWidth * scale);
  int scaledHeight = static_cast<int>(bgHeight * scale);

  float wrappedOffset = std::fmod(parallaxOffset, static_cast<float>(scaledWidth));
  if (wrappedOffset > 0) {
    wrappedOffset -= scaledWidth;
  }
  int offsetX = static_cast<int>(wrappedOffset);
  int numCopies = (windowWidth / scaledWidth) + 3;

  for (int i = -1; i < numCopies; i++) {
    m_renderer->drawTextureEx(background, offsetX + (i * scaledWidth), (windowHeight - scaledHeight) / 2, scaledWidth,
                              scaledHeight, 0.0f, false, false);
  }

  // Draw lobby title
  std::string title = "LOBBY";
  int titleWidth, titleHeight;
  m_renderer->getTextSize(font, title, titleWidth, titleHeight);
  float titleX = (windowWidth - titleWidth) / 2.0f;
  float titleY = windowHeight * 0.2f;
  m_renderer->drawText(font, title, static_cast<int>(titleX), static_cast<int>(titleY), {255, 255, 255, 255});

  // Draw ready status
  std::string statusText = m_ready ? "Ready! Waiting for others..." : "Press X to be ready";
  int statusWidth, statusHeight;
  m_renderer->getTextSize(font, statusText, statusWidth, statusHeight);
  float statusX = (windowWidth - statusWidth) / 2.0f;
  float statusY = windowHeight * 0.5f;
  Color statusColor = m_ready ? Color{0, 255, 0, 255} : Color{255, 255, 255, 255};
  m_renderer->drawText(font, statusText, static_cast<int>(statusX), static_cast<int>(statusY), statusColor);

  // Draw start instruction (only if ready)
  if (m_ready) {
    std::string startText = "Press ENTER to start";
    int startWidth, startHeight;
    m_renderer->getTextSize(font, startText, startWidth, startHeight);
    float startX = (windowWidth - startWidth) / 2.0f;
    float startY = windowHeight * 0.65f;
    m_renderer->drawText(font, startText, static_cast<int>(startX), static_cast<int>(startY), {255, 255, 100, 255});
  }
}
