/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Menu.hpp
*/

#include "../../client/interface/IRenderer.hpp"
#include <memory>

class Menu
{
public:
  Menu(std::shared_ptr<IRenderer> renderer) : m_renderer(renderer) {};
  ~Menu();
  bool init();
  void update(float deltaTime);
  void render();

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *font;
  void *menuInterface;
  void *background;
  float parallaxOffset = 0.0f;
  float blinkTimer = 0.0f;
  float blinkSpeed = 2.0f;
};
