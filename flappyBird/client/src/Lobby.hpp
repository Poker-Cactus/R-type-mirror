/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Lobby.hpp
*/

#pragma once
#include "../../client/interface/IRenderer.hpp"
#include <memory>

class Lobby
{
public:
  Lobby(std::shared_ptr<IRenderer> renderer) : m_renderer(renderer) {};
  ~Lobby();
  bool init();
  void update(float deltaTime);
  void render();

  bool isReady() const { return m_ready; }
  void setReady(bool ready) { m_ready = ready; }

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *font;
  void *background;
  float parallaxOffset = 0.0f;
  bool m_ready = false;
};
