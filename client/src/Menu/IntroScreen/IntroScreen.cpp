/**
 * @file IntroScreen.cpp
 * @brief Intro screen implementation with moon zoom animation
 */

#include "IntroScreen.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <cmath>

void IntroScreen::init(IRenderer *renderer)
{
  try {
    const int fontSize = 24;
    m_backgroundTexture = renderer->loadTexture("client/assets/background/starfield.png");
    m_font = renderer->loadFont("client/assets/font.opf/r-type.otf", fontSize);
    m_planet = renderer->loadTexture("client/assets/moon-pack/moon1.png");
  } catch (const std::exception &e) {
    m_backgroundTexture = nullptr;
    m_font = nullptr;
    m_planet = nullptr;
  }
}

void IntroScreen::render(int winWidth, int winHeight, IRenderer *renderer)
{
  // Update zoom animation
  if (m_isZooming) {
    m_zoomTimer += renderer->getDeltaTime();

    float progress = m_zoomTimer / m_zoomDuration;

    if (progress >= 1.0f) {
      m_isZooming = false;
      m_zoomTimer = 0.0f;
      m_zoomScale = 0.3f;
      m_isComplete = true;
      return;
    }

    m_zoomScale = 0.3f + (progress * progress * 9.7f);
  }

  // Scroll background
  m_backgroundOffsetX += renderer->getDeltaTime() * 20.0f;
  if (m_backgroundOffsetX >= winWidth) {
    m_backgroundOffsetX = 0.0f;
  }

  // Render background
  if (m_backgroundTexture != nullptr) {
    renderer->drawTextureEx(m_backgroundTexture, static_cast<int>(m_backgroundOffsetX), 0, winWidth, winHeight, 0.0,
                            false, false);
    renderer->drawTextureEx(m_backgroundTexture, static_cast<int>(m_backgroundOffsetX - winWidth), 0, winWidth,
                            winHeight, 0.0, false, false);
  }

  // Render planet with zoom effect
  if (m_planet != nullptr) {
    int planetWidth, planetHeight;
    renderer->getTextureSize(m_planet, planetWidth, planetHeight);

    float scale = static_cast<float>(std::min(winWidth, winHeight)) * m_zoomScale /
                  static_cast<float>(std::max(planetWidth, planetHeight));

    int scaledWidth = static_cast<int>(planetWidth * scale);
    int scaledHeight = static_cast<int>(planetHeight * scale);

    int planetX = (winWidth - scaledWidth) / 2;
    int planetY = (winHeight - scaledHeight) / 2;

    renderer->drawTextureEx(m_planet, planetX, planetY, scaledWidth, scaledHeight, 0.0, false, false);
  }

  // Render blinking "Press enter" text (only if not zooming)
  if (m_font != nullptr && !m_isZooming) {
    m_blinkTimer += renderer->getDeltaTime();
    float opacity = (std::sin(m_blinkTimer * 3.5f) + 1.0f) / 2.0f;
    int alpha = static_cast<int>(50 + opacity * 205);

    std::string text = "Press enter to start ...";
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(m_font, text, textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight - textHeight) / 1.1;

    renderer->drawText(m_font, text, x, y, {255, 255, 255, static_cast<unsigned char>(alpha)});
  }
}

bool IntroScreen::process(IRenderer *renderer)
{
  if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN) && !m_isZooming) {
    m_isZooming = true;
    return false;
  }
  return m_isComplete;
}
