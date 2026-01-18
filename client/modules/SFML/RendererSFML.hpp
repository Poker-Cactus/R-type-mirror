/**
 * @file RendererSFML.hpp
 * @brief SFML renderer implementation.
 */

#pragma once
#include "../../interface/IRenderer.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <map>
#include <memory>
#include <vector>

// Default configuration values for the SFML renderer
inline constexpr int kDefaultWindowWidth = 800;
inline constexpr int kDefaultWindowHeight = 600;
inline constexpr int kDefaultColorAlpha = 255;
inline constexpr int kDefaultTargetFPS = 60;
inline const Color kDefaultClearColor{.r = 0, .g = 0, .b = 0, .a = kDefaultColorAlpha};

/**
 * @class RendererSFML
 * @brief SFML-based implementation of the renderer interfaces.
 */
class RendererSFML : public IRenderer
{
public:
  /** @brief Create an SFML renderer with a given window size. */
  RendererSFML(int width = kDefaultWindowWidth, int height = kDefaultWindowHeight);
  ~RendererSFML() override;

  // === IRender ===
  /// @copydoc IRender::clear
  void clear(const Color &color = kDefaultClearColor) override;
  /// @copydoc IRender::present
  void present() override;

  // === IWindow ===
  /// @copydoc IWindow::getWindowWidth
  [[nodiscard]] int getWindowWidth() const override { return windowWidth; }
  /// @copydoc IWindow::getWindowHeight
  [[nodiscard]] int getWindowHeight() const override { return windowHeight; }
  /// @copydoc IWindow::setWindowSize
  void setWindowSize(int width, int height) override;
  /// @copydoc IWindow::setWindowTitle
  void setWindowTitle(const std::string &title) override;
  /// @copydoc IWindow::setFullscreen
  void setFullscreen(bool fullscreen) override;
  /// @copydoc IWindow::isFullscreen
  bool isFullscreen() const override;
  /// @copydoc IWindow::pollEvents
  bool pollEvents() override;

  // === IInput ===
  /// @copydoc IInput::isKeyPressed
  bool isKeyPressed(int keycode) override;
  /// @copydoc IInput::isKeyReleased
  bool isKeyReleased(int keycode) override;
  /// @copydoc IInput::isKeyJustPressed
  bool isKeyJustPressed(int keycode) override;
  /// @copydoc IInput::getMousePosition
  void getMousePosition(int &outX, int &outY) override;
  /// @copydoc IInput::isMouseButtonPressed
  bool isMouseButtonPressed(int button) override;
  /// @copydoc IInput::getNumGamepads
  int getNumGamepads() override;
  /// @copydoc IInput::isGamepadButtonPressed
  bool isGamepadButtonPressed(int gamepadIndex, int button) override;
  /// @copydoc IInput::getGamepadAxis
  float getGamepadAxis(int gamepadIndex, int axis) override;

  // === ITexture ===
  /// @copydoc ITexture::loadTexture
  void *loadTexture(const std::string &filepath) override;
  /// @copydoc ITexture::freeTexture
  void freeTexture(void *texture) override;
  /// @copydoc ITexture::getTextureSize
  void getTextureSize(void *texture, int &width, int &height) override;
  /// @copydoc ITexture::drawTexture
  void drawTexture(void *texture, int posX, int posY) override;
  /// @copydoc ITexture::drawTextureRegion
  void drawTextureRegion(void *texture, const Rect &src, const Rect &dst) override;
  /// @copydoc ITexture::drawTextureRegionEx
  void drawTextureRegionEx(void *texture, const Rect &src, const Rect &dst, double angle, bool flipX,
                           bool flipY) override;
  /// @copydoc ITexture::drawTextureEx
  void drawTextureEx(void *texture, int posX, int posY, int width, int height, double angle, bool flipX,
                     bool flipY) override;

  // === IText ===
  /// @copydoc IText::loadFont
  void *loadFont(const std::string &filepath, int fontSize) override;
  /// @copydoc IText::freeFont
  void freeFont(void *font) override;
  /// @copydoc IText::drawText
  void drawText(void *font, const std::string &text, int posX, int posY, const Color &color) override;
  /// @copydoc IText::getTextSize
  void getTextSize(void *font, const std::string &text, int &width, int &height) override;

  // === IAudio ===
  /// @copydoc IAudio::loadSound
  void *loadSound(const std::string &filepath) override;
  /// @copydoc IAudio::loadMusic
  void *loadMusic(const std::string &filepath) override;
  /// @copydoc IAudio::playSound
  void playSound(void *sound, int loops) override;
  /// @copydoc IAudio::playMusic
  void playMusic(void *music, int loops) override;
  /// @copydoc IAudio::pauseMusic
  void pauseMusic() override;
  /// @copydoc IAudio::resumeMusic
  void resumeMusic() override;
  /// @copydoc IAudio::stopMusic
  void stopMusic() override;
  /// @copydoc IAudio::setSoundVolume
  void setSoundVolume(int volume) override;
  /// @copydoc IAudio::setMusicVolume
  void setMusicVolume(int volume) override;
  /// @copydoc IAudio::freeSound
  void freeSound(void *sound) override;
  /// @copydoc IAudio::freeMusic
  void freeMusic(void *music) override;
  /// @copydoc IAudio::isMusicPlaying
  bool isMusicPlaying() override;

  // === IShape ===
  /// @copydoc IShape::drawRect
  void drawRect(int posX, int posY, int width, int height, const Color &color) override;
  /// @copydoc IShape::drawRectOutline
  void drawRectOutline(int posX, int posY, int width, int height, const Color &color) override;
  /// @copydoc IShape::drawLine
  void drawLine(int startX, int startY, int endX, int endY, const Color &color) override;
  /// @copydoc IShape::drawCircle
  void drawCircle(const Circle &circle, const Color &color) override;
  /// @copydoc IShape::drawCircleFilled
  void drawCircleFilled(const Circle &circle, const Color &color) override;
  /// @copydoc IShape::drawPoint
  void drawPoint(int posX, int posY, const Color &color) override;

  // === ICamera ===
  /// @copydoc ICamera::setViewport
  void setViewport(int posX, int posY, int width, int height) override;
  /// @copydoc ICamera::resetViewport
  void resetViewport() override;
  /// @copydoc ICamera::setCameraOffset
  void setCameraOffset(int offsetX, int offsetY) override;
  /// @copydoc ICamera::getCameraOffset
  void getCameraOffset(int &outOffsetX, int &outOffsetY) const override;

  // === ITime ===
  /// @copydoc ITime::getDeltaTime
  [[nodiscard]] float getDeltaTime() const override;
  /// @copydoc ITime::getFPS
  [[nodiscard]] int getFPS() const override;
  /// @copydoc ITime::setTargetFPS
  void setTargetFPS(int fps) override;
  /// @copydoc ITime::setVSync
  void setVSync(bool enabled) override;

  // === ICollision ===
  /// @copydoc ICollision::checkCollisionRects
  bool checkCollisionRects(int leftX, int leftY, int leftWidth, int leftHeight, int rightX, int rightY, int rightWidth,
                           int rightHeight) override;
  /// @copydoc ICollision::checkCollisionCircles
  bool checkCollisionCircles(const Circle &circle1, const Circle &circle2) override;
  /// @copydoc ICollision::checkPointInRect
  bool checkPointInRect(int pointX, int pointY, int rectX, int rectY, int rectW, int rectH) override;

private:
  sf::RenderWindow window;
  int windowWidth;
  int windowHeight;
  bool fullscreen = false;

  // Timing
  sf::Clock clock;
  float deltaTime = 0.0F;
  int targetFPS = kDefaultTargetFPS;
  int currentFPS = 0;
  sf::Time frameTime;

  // Camera
  int cameraOffsetX = 0;
  int cameraOffsetY = 0;
  sf::View cameraView;

  // Input state
  std::map<int, bool> keyStates;
  std::map<int, bool> previousKeyStates;
  std::map<int, bool> mouseButtonStates;
  int mouseX = 0;
  int mouseY = 0;

  // Resources
  std::vector<std::unique_ptr<sf::Texture>> textures;
  std::vector<std::unique_ptr<sf::Font>> fonts;
  std::vector<std::unique_ptr<sf::SoundBuffer>> soundBuffers;
  std::vector<std::unique_ptr<sf::Sound>> sounds;
  std::vector<std::unique_ptr<sf::Music>> musics;

  // Audio
  sf::Music *currentMusic = nullptr;
};
