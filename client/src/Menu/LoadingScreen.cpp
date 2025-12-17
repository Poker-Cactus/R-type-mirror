/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** LoadingScreen.cpp - Implémentation de la classe interne Menu::LoadingScreen
*/

#include "LoadingScreen.hpp"
#include <cmath>

LoadingScreen::LoadingScreen(IRenderer *renderer, void *font) : renderer(renderer), font(font) {}

void LoadingScreen::start()
{
  active = true;
  timer = 0.0f;
  rotation = 0.0f;
}

void LoadingScreen::stop()
{
  active = false;
  timer = 0.0f;
  rotation = 0.0f;
}

bool LoadingScreen::update(int winWidth, int winHeight)
{
  if (!active) {
    return true;
  }

  timer += renderer->getDeltaTime();

  if (timer >= duration) {
    stop();
    return true;
  }
  renderer->drawRect(0, 0, winWidth, winHeight, {0, 0, 0, 255});

  rotation += renderer->getDeltaTime() * 360.0f;
  if (rotation >= 360.0f) {
    rotation -= 360.0f;
  }

  if (font != nullptr) {
    std::string text = "Loading...";
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(font, text, textWidth, textHeight);

    int x = winWidth - textWidth - 50;
    int y = winHeight - textHeight - 30;

    float opacity = (std::sin(rotation * 3.14159f / 180.0f) + 1.0f) / 2.0f;
    int alpha = static_cast<int>(100 + opacity * 155);

    renderer->drawText(font, text, x, y, {255, 255, 255, static_cast<unsigned char>(alpha)});
  }

  return false;
}

bool LoadingScreen::isActive() const
{
  return active;
}

void LoadingScreen::setDuration(float newDuration)
{
  duration = newDuration;
}
