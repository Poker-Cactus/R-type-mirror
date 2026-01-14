/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** RendererSFML.cpp
*/

#include "RendererSFML.hpp"
#include "../../interface/Color.hpp"
#include "../../interface/KeyCodes.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace
{
constexpr float kMillisecondsPerSecond = 1000.0F;
constexpr float kGamepadAxisMax = 100.0F; // SFML uses -100 to 100 range
} // anonymous namespace

// Map SFML keycodes to generic keycodes
static int mapSFMLKeyToGeneric(sf::Keyboard::Key sfmlKey)
{
  switch (sfmlKey) {
  case sf::Keyboard::Return:
    return KeyCode::KEY_RETURN;
  case sf::Keyboard::Escape:
    return KeyCode::KEY_ESCAPE;
  case sf::Keyboard::Backspace:
    return KeyCode::KEY_BACKSPACE;
  case sf::Keyboard::Tab:
    return KeyCode::KEY_TAB;
  case sf::Keyboard::Space:
    return KeyCode::KEY_SPACE;
  case sf::Keyboard::Delete:
    return KeyCode::KEY_DELETE;

  case sf::Keyboard::Up:
    return KeyCode::KEY_UP;
  case sf::Keyboard::Down:
    return KeyCode::KEY_DOWN;
  case sf::Keyboard::Left:
    return KeyCode::KEY_LEFT;
  case sf::Keyboard::Right:
    return KeyCode::KEY_RIGHT;

  case sf::Keyboard::A:
    return KeyCode::KEY_A;
  case sf::Keyboard::B:
    return KeyCode::KEY_B;
  case sf::Keyboard::C:
    return KeyCode::KEY_C;
  case sf::Keyboard::D:
    return KeyCode::KEY_D;
  case sf::Keyboard::E:
    return KeyCode::KEY_E;
  case sf::Keyboard::F:
    return KeyCode::KEY_F;
  case sf::Keyboard::G:
    return KeyCode::KEY_G;
  case sf::Keyboard::H:
    return KeyCode::KEY_H;
  case sf::Keyboard::I:
    return KeyCode::KEY_I;
  case sf::Keyboard::J:
    return KeyCode::KEY_J;
  case sf::Keyboard::K:
    return KeyCode::KEY_K;
  case sf::Keyboard::L:
    return KeyCode::KEY_L;
  case sf::Keyboard::M:
    return KeyCode::KEY_M;
  case sf::Keyboard::N:
    return KeyCode::KEY_N;
  case sf::Keyboard::O:
    return KeyCode::KEY_O;
  case sf::Keyboard::P:
    return KeyCode::KEY_P;
  case sf::Keyboard::Q:
    return KeyCode::KEY_Q;
  case sf::Keyboard::R:
    return KeyCode::KEY_R;
  case sf::Keyboard::S:
    return KeyCode::KEY_S;
  case sf::Keyboard::T:
    return KeyCode::KEY_T;
  case sf::Keyboard::U:
    return KeyCode::KEY_U;
  case sf::Keyboard::V:
    return KeyCode::KEY_V;
  case sf::Keyboard::W:
    return KeyCode::KEY_W;
  case sf::Keyboard::X:
    return KeyCode::KEY_X;
  case sf::Keyboard::Y:
    return KeyCode::KEY_Y;
  case sf::Keyboard::Z:
    return KeyCode::KEY_Z;

  case sf::Keyboard::Num0:
    return KeyCode::KEY_0;
  case sf::Keyboard::Num1:
    return KeyCode::KEY_1;
  case sf::Keyboard::Num2:
    return KeyCode::KEY_2;
  case sf::Keyboard::Num3:
    return KeyCode::KEY_3;
  case sf::Keyboard::Num4:
    return KeyCode::KEY_4;
  case sf::Keyboard::Num5:
    return KeyCode::KEY_5;
  case sf::Keyboard::Num6:
    return KeyCode::KEY_6;
  case sf::Keyboard::Num7:
    return KeyCode::KEY_7;
  case sf::Keyboard::Num8:
    return KeyCode::KEY_8;
  case sf::Keyboard::Num9:
    return KeyCode::KEY_9;

  default:
    return KeyCode::KEY_UNKNOWN;
  }
}

// Map generic keycodes to SFML keycodes
static sf::Keyboard::Key mapGenericKeyToSFML(int genericKey)
{
  switch (genericKey) {
  case KeyCode::KEY_RETURN:
    return sf::Keyboard::Return;
  case KeyCode::KEY_ESCAPE:
    return sf::Keyboard::Escape;
  case KeyCode::KEY_BACKSPACE:
    return sf::Keyboard::Backspace;
  case KeyCode::KEY_TAB:
    return sf::Keyboard::Tab;
  case KeyCode::KEY_SPACE:
    return sf::Keyboard::Space;
  case KeyCode::KEY_DELETE:
    return sf::Keyboard::Delete;

  case KeyCode::KEY_UP:
    return sf::Keyboard::Up;
  case KeyCode::KEY_DOWN:
    return sf::Keyboard::Down;
  case KeyCode::KEY_LEFT:
    return sf::Keyboard::Left;
  case KeyCode::KEY_RIGHT:
    return sf::Keyboard::Right;

  case KeyCode::KEY_A:
    return sf::Keyboard::A;
  case KeyCode::KEY_B:
    return sf::Keyboard::B;
  case KeyCode::KEY_C:
    return sf::Keyboard::C;
  case KeyCode::KEY_D:
    return sf::Keyboard::D;
  case KeyCode::KEY_E:
    return sf::Keyboard::E;
  case KeyCode::KEY_F:
    return sf::Keyboard::F;
  case KeyCode::KEY_G:
    return sf::Keyboard::G;
  case KeyCode::KEY_H:
    return sf::Keyboard::H;
  case KeyCode::KEY_I:
    return sf::Keyboard::I;
  case KeyCode::KEY_J:
    return sf::Keyboard::J;
  case KeyCode::KEY_K:
    return sf::Keyboard::K;
  case KeyCode::KEY_L:
    return sf::Keyboard::L;
  case KeyCode::KEY_M:
    return sf::Keyboard::M;
  case KeyCode::KEY_N:
    return sf::Keyboard::N;
  case KeyCode::KEY_O:
    return sf::Keyboard::O;
  case KeyCode::KEY_P:
    return sf::Keyboard::P;
  case KeyCode::KEY_Q:
    return sf::Keyboard::Q;
  case KeyCode::KEY_R:
    return sf::Keyboard::R;
  case KeyCode::KEY_S:
    return sf::Keyboard::S;
  case KeyCode::KEY_T:
    return sf::Keyboard::T;
  case KeyCode::KEY_U:
    return sf::Keyboard::U;
  case KeyCode::KEY_V:
    return sf::Keyboard::V;
  case KeyCode::KEY_W:
    return sf::Keyboard::W;
  case KeyCode::KEY_X:
    return sf::Keyboard::X;
  case KeyCode::KEY_Y:
    return sf::Keyboard::Y;
  case KeyCode::KEY_Z:
    return sf::Keyboard::Z;

  case KeyCode::KEY_0:
    return sf::Keyboard::Num0;
  case KeyCode::KEY_1:
    return sf::Keyboard::Num1;
  case KeyCode::KEY_2:
    return sf::Keyboard::Num2;
  case KeyCode::KEY_3:
    return sf::Keyboard::Num3;
  case KeyCode::KEY_4:
    return sf::Keyboard::Num4;
  case KeyCode::KEY_5:
    return sf::Keyboard::Num5;
  case KeyCode::KEY_6:
    return sf::Keyboard::Num6;
  case KeyCode::KEY_7:
    return sf::Keyboard::Num7;
  case KeyCode::KEY_8:
    return sf::Keyboard::Num8;
  case KeyCode::KEY_9:
    return sf::Keyboard::Num9;

    return sf::Keyboard::F12;

  default:
    return sf::Keyboard::Unknown;
  }
}

RendererSFML::RendererSFML(int width, int height)
    : window(sf::VideoMode(width, height), "SFML Window"), windowWidth(width), windowHeight(height)
{
  std::cout << "[RendererSFML] Creating SFML window " << width << "x" << height << std::endl;
  window.setFramerateLimit(targetFPS);
  cameraView = window.getDefaultView();
  std::cout << "[RendererSFML] SFML window created successfully" << std::endl;
}

RendererSFML::~RendererSFML()
{
  if (currentMusic) {
    currentMusic->stop();
  }
}

// === IRender ===
void RendererSFML::clear(const Color &color)
{
  window.clear(sf::Color(color.r, color.g, color.b, color.a));
}

void RendererSFML::present()
{
  std::cout << "[RendererSFML] Presenting frame" << std::endl;
  window.display();

  // Update timing
  frameTime = clock.restart();
  deltaTime = frameTime.asSeconds();

  // Calculate FPS
  static sf::Time fpsTimer;
  static int frameCount = 0;
  fpsTimer += frameTime;
  frameCount++;
  if (fpsTimer.asSeconds() >= 1.0f) {
    currentFPS = frameCount;
    frameCount = 0;
    fpsTimer = sf::Time::Zero;
  }
}

// === IWindow ===
void RendererSFML::setWindowSize(int width, int height)
{
  windowWidth = width;
  windowHeight = height;
  window.setSize(sf::Vector2u(width, height));
  cameraView.setSize(width, height);
  window.setView(cameraView);
}

void RendererSFML::setWindowTitle(const std::string &title)
{
  window.setTitle(title);
}

void RendererSFML::setFullscreen(bool fullscreen)
{
  this->fullscreen = fullscreen;
  if (fullscreen) {
    window.create(sf::VideoMode::getDesktopMode(), "SFML Window", sf::Style::Fullscreen);
  } else {
    window.create(sf::VideoMode(windowWidth, windowHeight), "SFML Window", sf::Style::Default);
  }
  window.setFramerateLimit(targetFPS);
  cameraView = window.getDefaultView();

  // Update windowWidth and windowHeight to match the actual window size
  auto size = window.getSize();
  windowWidth = size.x;
  windowHeight = size.y;
}

bool RendererSFML::isFullscreen() const
{
  return fullscreen;
}

bool RendererSFML::pollEvents()
{
  sf::Event event;
  bool hasEvents = false;

  // Update previous key states
  previousKeyStates = keyStates;

  while (window.pollEvent(event)) {
    hasEvents = true;

    if (event.type == sf::Event::Closed) {
      window.close();
      return false; // Window closed, stop the game loop
    } else if (event.type == sf::Event::KeyPressed) {
      int key = mapSFMLKeyToGeneric(event.key.code);
      keyStates[key] = true;
      if (event.key.code == sf::Keyboard::Escape) {
        return false; // ESC pressed, stop the game loop
      }
    } else if (event.type == sf::Event::KeyReleased) {
      int key = mapSFMLKeyToGeneric(event.key.code);
      keyStates[key] = false;
    } else if (event.type == sf::Event::MouseButtonPressed) {
      mouseButtonStates[event.mouseButton.button] = true;
    } else if (event.type == sf::Event::MouseButtonReleased) {
      mouseButtonStates[event.mouseButton.button] = false;
    } else if (event.type == sf::Event::MouseMoved) {
      mouseX = event.mouseMove.x;
      mouseY = event.mouseMove.y;
    }
  }

  // Return true if window is still open (continue the game loop)
  bool result = window.isOpen();
  return result;
}

// === IInput ===
bool RendererSFML::isKeyPressed(int keycode)
{
  sf::Keyboard::Key sfmlKey = mapGenericKeyToSFML(keycode);
  return sf::Keyboard::isKeyPressed(sfmlKey);
}

bool RendererSFML::isKeyReleased(int keycode)
{
  return !isKeyPressed(keycode);
}

bool RendererSFML::isKeyJustPressed(int keycode)
{
  bool current = keyStates[keycode];
  bool previous = previousKeyStates[keycode];
  return current && !previous;
}

void RendererSFML::getMousePosition(int &outX, int &outY)
{
  sf::Vector2i pos = sf::Mouse::getPosition(window);
  outX = pos.x;
  outY = pos.y;
}

bool RendererSFML::isMouseButtonPressed(int button)
{
  return sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(button));
}

int RendererSFML::getNumGamepads()
{
  return sf::Joystick::Count;
}

bool RendererSFML::isGamepadButtonPressed(int gamepadIndex, int button)
{
  return sf::Joystick::isButtonPressed(gamepadIndex, button);
}

float RendererSFML::getGamepadAxis(int gamepadIndex, int axis)
{
  return sf::Joystick::getAxisPosition(gamepadIndex, static_cast<sf::Joystick::Axis>(axis)) / kGamepadAxisMax;
}

// === ITexture ===
void *RendererSFML::loadTexture(const std::string &filepath)
{
  auto texture = std::make_unique<sf::Texture>();
  if (!texture->loadFromFile(filepath)) {
    std::cerr << "Failed to load texture: " << filepath << std::endl;
    return nullptr;
  }
  textures.push_back(std::move(texture));
  return textures.back().get();
}

void RendererSFML::freeTexture(void *texture)
{
  // SFML textures are automatically managed
}

void RendererSFML::getTextureSize(void *texture, int &width, int &height)
{
  if (texture) {
    sf::Texture *tex = static_cast<sf::Texture *>(texture);
    sf::Vector2u size = tex->getSize();
    width = size.x;
    height = size.y;
  }
}

void RendererSFML::drawTexture(void *texture, int posX, int posY)
{
  if (!texture)
    return;

  sf::Texture *tex = static_cast<sf::Texture *>(texture);
  sf::Sprite sprite(*tex);
  sprite.setPosition(posX + cameraOffsetX, posY + cameraOffsetY);
  window.draw(sprite);
}

void RendererSFML::drawTextureRegion(void *texture, const Rect &src, const Rect &dst)
{
  if (!texture)
    return;

  sf::Texture *tex = static_cast<sf::Texture *>(texture);
  sf::Sprite sprite(*tex);
  sprite.setTextureRect(sf::IntRect(src.x, src.y, src.width, src.height));
  sprite.setPosition(dst.x + cameraOffsetX, dst.y + cameraOffsetY);
  sprite.setScale(static_cast<float>(dst.width) / src.width, static_cast<float>(dst.height) / src.height);
  window.draw(sprite);
}

void RendererSFML::drawTextureRegionEx(void *texture, const Rect &src, const Rect &dst, double angle, bool flipX,
                                       bool flipY)
{
  if (!texture)
    return;

  sf::Texture *tex = static_cast<sf::Texture *>(texture);
  sf::Sprite sprite(*tex);
  sprite.setTextureRect(sf::IntRect(src.x, src.y, src.width, src.height));
  sprite.setPosition(dst.x + cameraOffsetX, dst.y + cameraOffsetY);
  sprite.setScale(static_cast<float>(dst.width) / src.width, static_cast<float>(dst.height) / src.height);
  sprite.setRotation(static_cast<float>(angle));

  sf::Vector2f scale = sprite.getScale();
  if (flipX)
    scale.x = -std::abs(scale.x);
  if (flipY)
    scale.y = -std::abs(scale.y);
  sprite.setScale(scale);

  window.draw(sprite);
}

void RendererSFML::drawTextureEx(void *texture, int posX, int posY, int width, int height, double angle, bool flipX,
                                 bool flipY)
{
  if (!texture)
    return;

  sf::Texture *tex = static_cast<sf::Texture *>(texture);
  sf::Sprite sprite(*tex);
  sprite.setPosition(posX + cameraOffsetX, posY + cameraOffsetY);
  sprite.setScale(static_cast<float>(width) / tex->getSize().x, static_cast<float>(height) / tex->getSize().y);
  sprite.setRotation(static_cast<float>(angle));

  sf::Vector2f scale = sprite.getScale();
  if (flipX)
    scale.x = -std::abs(scale.x);
  if (flipY)
    scale.y = -std::abs(scale.y);
  sprite.setScale(scale);

  window.draw(sprite);
}

// === IText ===
void *RendererSFML::loadFont(const std::string &filepath, int fontSize)
{
  auto font = std::make_unique<sf::Font>();
  if (!font->loadFromFile(filepath)) {
    std::cerr << "Failed to load font: " << filepath << std::endl;
    return nullptr;
  }
  fonts.push_back(std::move(font));
  return fonts.back().get();
}

void RendererSFML::freeFont(void *font)
{
  // SFML fonts are automatically managed
}

void RendererSFML::drawText(void *font, const std::string &text, int posX, int posY, const Color &color)
{
  if (!font)
    return;

  sf::Font *f = static_cast<sf::Font *>(font);
  sf::Text textObj(text, *f);
  textObj.setPosition(posX + cameraOffsetX, posY + cameraOffsetY);
  textObj.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
  window.draw(textObj);
}

void RendererSFML::getTextSize(void *font, const std::string &text, int &width, int &height)
{
  if (!font)
    return;

  sf::Font *f = static_cast<sf::Font *>(font);
  sf::Text textObj(text, *f);
  sf::FloatRect bounds = textObj.getLocalBounds();
  width = static_cast<int>(bounds.width);
  height = static_cast<int>(bounds.height);
}

// === IAudio ===
void *RendererSFML::loadSound(const std::string &filepath)
{
  auto soundBuffer = std::make_unique<sf::SoundBuffer>();
  if (!soundBuffer->loadFromFile(filepath)) {
    std::cerr << "Failed to load sound: " << filepath << std::endl;
    return nullptr;
  }
  soundBuffers.push_back(std::move(soundBuffer));

  auto sound = std::make_unique<sf::Sound>();
  sound->setBuffer(*soundBuffers.back());
  sounds.push_back(std::move(sound));

  return sounds.back().get();
}

void *RendererSFML::loadMusic(const std::string &filepath)
{
  auto music = std::make_unique<sf::Music>();
  if (!music->openFromFile(filepath)) {
    std::cerr << "Failed to load music: " << filepath << std::endl;
    return nullptr;
  }
  musics.push_back(std::move(music));
  return musics.back().get();
}

void RendererSFML::playSound(void *sound, int loops)
{
  if (!sound)
    return;

  sf::Sound *s = static_cast<sf::Sound *>(sound);
  s->setLoop(loops != 0);
  s->play();
}

void RendererSFML::playMusic(void *music, int loops)
{
  if (!music)
    return;

  if (currentMusic) {
    currentMusic->stop();
  }

  sf::Music *m = static_cast<sf::Music *>(music);
  m->setLoop(loops != 0);
  m->play();
  currentMusic = m;
}

void RendererSFML::pauseMusic()
{
  if (currentMusic) {
    currentMusic->pause();
  }
}

void RendererSFML::resumeMusic()
{
  if (currentMusic) {
    currentMusic->play();
  }
}

void RendererSFML::stopMusic()
{
  if (currentMusic) {
    currentMusic->stop();
    currentMusic = nullptr;
  }
}

void RendererSFML::setSoundVolume(int volume)
{
  for (auto &sound : sounds) {
    if (sound) {
      sound->setVolume(volume);
    }
  }
}

void RendererSFML::setMusicVolume(int volume)
{
  if (currentMusic) {
    currentMusic->setVolume(volume);
  }
}

void RendererSFML::freeSound(void *sound)
{
  // SFML sounds are automatically managed
}

void RendererSFML::freeMusic(void *music)
{
  // SFML music is automatically managed
}

bool RendererSFML::isMusicPlaying()
{
  return currentMusic && currentMusic->getStatus() == sf::Music::Playing;
}

// === IShape ===
void RendererSFML::drawRect(int posX, int posY, int width, int height, const Color &color)
{
  sf::RectangleShape rect(sf::Vector2f(width, height));
  rect.setPosition(posX + cameraOffsetX, posY + cameraOffsetY);
  rect.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
  window.draw(rect);
}

void RendererSFML::drawRectOutline(int posX, int posY, int width, int height, const Color &color)
{
  sf::RectangleShape rect(sf::Vector2f(width, height));
  rect.setPosition(posX + cameraOffsetX, posY + cameraOffsetY);
  rect.setFillColor(sf::Color::Transparent);
  rect.setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
  rect.setOutlineThickness(1.0f);
  window.draw(rect);
}

void RendererSFML::drawLine(int startX, int startY, int endX, int endY, const Color &color)
{
  sf::Vertex line[] = {sf::Vertex(sf::Vector2f(startX + cameraOffsetX, startY + cameraOffsetY),
                                  sf::Color(color.r, color.g, color.b, color.a)),
                       sf::Vertex(sf::Vector2f(endX + cameraOffsetX, endY + cameraOffsetY),
                                  sf::Color(color.r, color.g, color.b, color.a))};
  window.draw(line, 2, sf::Lines);
}

void RendererSFML::drawCircle(const Circle &circle, const Color &color)
{
  sf::CircleShape shape(circle.radius);
  shape.setPosition(circle.centerX + cameraOffsetX - circle.radius, circle.centerY + cameraOffsetY - circle.radius);
  shape.setFillColor(sf::Color::Transparent);
  shape.setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
  shape.setOutlineThickness(1.0f);
  window.draw(shape);
}

void RendererSFML::drawCircleFilled(const Circle &circle, const Color &color)
{
  sf::CircleShape shape(circle.radius);
  shape.setPosition(circle.centerX + cameraOffsetX - circle.radius, circle.centerY + cameraOffsetY - circle.radius);
  shape.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
  window.draw(shape);
}

void RendererSFML::drawPoint(int posX, int posY, const Color &color)
{
  sf::Vertex point(sf::Vector2f(posX + cameraOffsetX, posY + cameraOffsetY),
                   sf::Color(color.r, color.g, color.b, color.a));
  window.draw(&point, 1, sf::Points);
}

// === ICamera ===
void RendererSFML::setViewport(int posX, int posY, int width, int height)
{
  sf::View viewport(sf::FloatRect(posX, posY, width, height));
  window.setView(viewport);
}

void RendererSFML::resetViewport()
{
  window.setView(cameraView);
}

void RendererSFML::setCameraOffset(int offsetX, int offsetY)
{
  cameraOffsetX = offsetX;
  cameraOffsetY = offsetY;
  cameraView.setCenter(windowWidth / 2.0f + offsetX, windowHeight / 2.0f + offsetY);
  window.setView(cameraView);
}

void RendererSFML::getCameraOffset(int &outOffsetX, int &outOffsetY) const
{
  outOffsetX = cameraOffsetX;
  outOffsetY = cameraOffsetY;
}

// === ITime ===
float RendererSFML::getDeltaTime() const
{
  return deltaTime;
}

int RendererSFML::getFPS() const
{
  return currentFPS;
}

void RendererSFML::setTargetFPS(int fps)
{
  targetFPS = fps;
  window.setFramerateLimit(fps);
}

void RendererSFML::setVSync(bool enabled)
{
  window.setVerticalSyncEnabled(enabled);
}

// === ICollision ===
bool RendererSFML::checkCollisionRects(int leftX, int leftY, int leftWidth, int leftHeight, int rightX, int rightY,
                                       int rightWidth, int rightHeight)
{
  return (leftX < rightX + rightWidth && leftX + leftWidth > rightX && leftY < rightY + rightHeight &&
          leftY + leftHeight > rightY);
}

bool RendererSFML::checkCollisionCircles(const Circle &circle1, const Circle &circle2)
{
  float dx = circle1.centerX - circle2.centerX;
  float dy = circle1.centerY - circle2.centerY;
  float distance = std::sqrt(dx * dx + dy * dy);
  return distance < (circle1.radius + circle2.radius);
}

bool RendererSFML::checkPointInRect(int pointX, int pointY, int rectX, int rectY, int rectW, int rectH)
{
  return (pointX >= rectX && pointX <= rectX + rectW && pointY >= rectY && pointY <= rectY + rectH);
}
