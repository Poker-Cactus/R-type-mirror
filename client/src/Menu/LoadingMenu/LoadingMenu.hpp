#pragma once
#include "../../../interface/IRenderer.hpp"
#include "../LoadingScreen.hpp"
#include "../MenuState.hpp"

class LoadingMenu
{
public:
  LoadingMenu(){};
  ~LoadingMenu(){};
  void init(std::shared_ptr<IRenderer> renderer);
  void render(int winWidth, int winHeight, std::shared_ptr<IRenderer> renderer, LoadingScreen *loadingScreen,
              MenuState *currentState);
  void process(std::shared_ptr<IRenderer> renderer);

private:
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
};