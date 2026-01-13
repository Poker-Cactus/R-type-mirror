#pragma once
#include "../../../interface/IRenderer.hpp"
// #include "../LoadingScreen.hpp"
#include "../MenuState.hpp"
#include <memory>

class LoadingMenu
{
public:
  LoadingMenu(std::shared_ptr<IRenderer> renderer): m_renderer(renderer){};
  ~LoadingMenu(){};
  void init();
  void render(int winWidth, int winHeight,
              MenuState *currentState);
  void process();

private:
  std::shared_ptr<IRenderer> m_renderer;
  void *backgroundTexture = nullptr;
  void *font = nullptr;
  void *title_font = nullptr;
  void *planet = nullptr;
  float blinkTimer = 0.0f;
  bool isZooming = false;
  float zoomTimer = 0.0f;
  float zoomScale = 0.3f;
  float zoomDuration = 0.8f;
  float backgroundOffsetX = 0.0f;
  float parallaxOffsetSky = 0.0f;

  void *music;
};