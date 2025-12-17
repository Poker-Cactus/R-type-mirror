/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Overlay implementation
*/

#include "Overlay.hpp"

Overlay::Overlay(IRenderer *renderer, std::uint8_t alpha)
    : m_renderer(renderer), m_red(0), m_green(0), m_blue(0), m_alpha(alpha)
{
}

Overlay::Overlay(IRenderer *renderer, const ColorRGBA &color)
    : m_renderer(renderer), m_red(color.red), m_green(color.green), m_blue(color.blue), m_alpha(color.alpha)
{
}

void Overlay::setColor(const ColorRGBA &color)
{
  m_red = color.red;
  m_green = color.green;
  m_blue = color.blue;
  m_alpha = color.alpha;
}

void Overlay::setAlpha(std::uint8_t alpha)
{
  m_alpha = alpha;
}

void Overlay::render()
{
  if (m_renderer == nullptr) {
    return;
  }

  int width = m_renderer->getWindowWidth();
  int height = m_renderer->getWindowHeight();

  Color overlayColor = {.r = m_red, .g = m_green, .b = m_blue, .a = m_alpha};
  m_renderer->drawRect(0, 0, width, height, overlayColor);
}
