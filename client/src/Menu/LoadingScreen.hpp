/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LoadingScreen.hpp
*/

#pragma once
#include "../../interface/IRenderer.hpp"
#include <memory>

class LoadingScreen
{
public:
  LoadingScreen(std::shared_ptr<IRenderer> renderer, void *font);
  ~LoadingScreen();

  void start();
  void stop();
  bool update(int winWidth, int winHeight);
  bool isActive() const;
  void setDuration(float duration);

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *m_font;
  bool active = false;
  float timer = 0.0f;
  float duration = 2.0f;
  float rotation = 0.0f;
};
