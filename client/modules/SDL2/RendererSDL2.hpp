#pragma once
#include "../../interface/IRenderer.hpp"
#include <SDL2/SDL.h>
#include <map>
#include <vector>

// Default configuration values for the SDL2 renderer
inline constexpr int kDefaultWindowWidth = 800;
inline constexpr int kDefaultWindowHeight = 600;
inline constexpr int kDefaultColorAlpha = 255;
inline constexpr int kDefaultTargetFPS = 60;
inline const Color kDefaultClearColor{.r = 0, .g = 0, .b = 0, .a = kDefaultColorAlpha};

class RendererSDL2 : public IRenderer
{
public:
  RendererSDL2(int width = kDefaultWindowWidth, int height = kDefaultWindowHeight);
  ~RendererSDL2() override;

  // === IRender ===
  void clear(const Color &color = kDefaultClearColor) override;
  void present() override;

  // === IWindow ===
  [[nodiscard]] int getWindowWidth() const override { return windowWidth; }
  [[nodiscard]] int getWindowHeight() const override { return windowHeight; }
  void setWindowSize(int width, int height) override;
  void setWindowTitle(const std::string &title) override;
  void setFullscreen(bool fullscreen) override;
  bool isFullscreen() const override;
  bool pollEvents() override;

  // === IInput ===
  bool isKeyPressed(int keycode) override;
  bool isKeyReleased(int keycode) override;
  bool isKeyJustPressed(int keycode) override;
  void getMousePosition(int &outX, int &outY) override;
  bool isMouseButtonPressed(int button) override;
  int getNumGamepads() override;
  bool isGamepadButtonPressed(int gamepadIndex, int button) override;
  float getGamepadAxis(int gamepadIndex, int axis) override;

  // === ITexture ===
  void *loadTexture(const std::string &filepath) override;
  void freeTexture(void *texture) override;
  void getTextureSize(void *texture, int &width, int &height) override;
  void drawTexture(void *texture, int posX, int posY) override;
  void drawTextureRegion(void *texture, const Rect &src, const Rect &dst) override;
  void drawTextureEx(void *texture, int posX, int posY, int width, int height, double angle, bool flipX, bool flipY) override;

  // === IText ===
  void *loadFont(const std::string &filepath, int fontSize) override;
  void freeFont(void *font) override;
  void drawText(void *font, const std::string &text, int posX, int posY, const Color &color) override;
  void getTextSize(void *font, const std::string &text, int &width, int &height) override;

  // === IAudio ===
  void *loadSound(const std::string &filepath) override;
  void *loadMusic(const std::string &filepath) override;
  void playSound(void *sound, int loops) override;
  void playMusic(void *music, int loops) override;
  void pauseMusic() override;
  void resumeMusic() override;
  void stopMusic() override;
  void setSoundVolume(int volume) override;
  void setMusicVolume(int volume) override;
  void freeSound(void *sound) override;
  void freeMusic(void *music) override;

  // === IShape ===
  void drawRect(int posX, int posY, int width, int height, const Color &color) override;
  void drawRectOutline(int posX, int posY, int width, int height, const Color &color) override;
  void drawLine(int startX, int startY, int endX, int endY, const Color &color) override;
  void drawCircle(const Circle &circle, const Color &color) override;
  void drawCircleFilled(const Circle &circle, const Color &color) override;
  void drawPoint(int posX, int posY, const Color &color) override;

  // === ICamera ===
  void setViewport(int posX, int posY, int width, int height) override;
  void resetViewport() override;
  void setCameraOffset(int offsetX, int offsetY) override;
  void getCameraOffset(int &outOffsetX, int &outOffsetY) const override;

  // === ITime ===
  [[nodiscard]] float getDeltaTime() const override;
  [[nodiscard]] int getFPS() const override;
  void setTargetFPS(int fps) override;
  void setVSync(bool enabled) override;

  // === ICollision ===
  bool checkCollisionRects(int leftX, int leftY, int leftWidth, int leftHeight, int rightX, int rightY, int rightWidth,
                           int rightHeight) override;
  bool checkCollisionCircles(const Circle &circle1, const Circle &circle2) override;
  bool checkPointInRect(int pointX, int pointY, int rectX, int rectY, int rectW, int rectH) override;

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  int windowWidth;
  int windowHeight;
  bool fullscreen = false;

  // Timing
  Uint64 lastFrameTime = 0;
  float deltaTime = 0.0F;
  int targetFPS = kDefaultTargetFPS;
  int currentFPS = 0;

  // Camera
  int cameraOffsetX = 0;
  int cameraOffsetY = 0;

  // Input state
  std::map<int, bool> keyStates;
  std::map<int, bool> previousKeyStates;
  std::map<int, bool> mouseButtonStates;
  int mouseX = 0;
  int mouseY = 0;

  // Gamepads
  std::vector<SDL_GameController *> gamepads;
};
