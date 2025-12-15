/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyRoomState implementation
*/

#include "LobbyRoomState.hpp"
#include "../../engineCore/include/ecs/ComponentSignature.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../interface/KeyCodes.hpp"
#include <iostream>

LobbyRoomState::LobbyRoomState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world)
    : renderer(renderer), world(world), background(nullptr), overlay(nullptr)
{
}

LobbyRoomState::~LobbyRoomState()
{
  cleanup();
}

bool LobbyRoomState::init()
{
  if (renderer == nullptr) {
    std::cerr << "LobbyRoomState: Renderer is null" << '\n';
    return false;
  }

  background = std::make_unique<ParallaxBackground>(renderer);
  if (!background->init()) {
    std::cerr << "LobbyRoomState: Failed to initialize parallax background" << '\n';
    return false;
  }

  constexpr std::uint8_t OVERLAY_ALPHA = 180;
  overlay = std::make_unique<Overlay>(renderer, OVERLAY_ALPHA);

  loadSpriteTextures();

  constexpr int LOBBY_FONT_SIZE = 32;
  try {
    m_lobbyFont = renderer->loadFont("client/assets/font.opf/r-type.otf", LOBBY_FONT_SIZE);
  } catch (const std::exception &e) {
    std::cerr << "LobbyRoomState: Failed to load font: " << e.what() << '\n';
    m_lobbyFont = nullptr;
  }

  return true;
}

void LobbyRoomState::update([[maybe_unused]] float deltaTime)
{
  if (background) {
    background->update(deltaTime);
  }
}

void LobbyRoomState::render()
{
  if (!world || renderer == nullptr) {
    return;
  }

  if (background) {
    background->render();
  }

  if (overlay) {
    overlay->render();
  }

  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<ecs::Transform>());
  sig.set(ecs::getComponentId<ecs::Sprite>());

  std::vector<ecs::Entity> entities;
  world->getEntitiesWithSignature(sig, entities);

  constexpr int SPRITE_SOURCE_X = 0;
  constexpr int SPRITE_SOURCE_Y = 0;
  constexpr int SPRITE_SOURCE_WIDTH = 350;
  constexpr int SPRITE_SOURCE_HEIGHT = 150;

  for (auto entity : entities) {
    const auto &transform = world->getComponent<ecs::Transform>(entity);
    const auto &sprite = world->getComponent<ecs::Sprite>(entity);

    auto textureIt = m_spriteTextures.find(sprite.spriteId);

    if (textureIt != m_spriteTextures.end() && textureIt->second != nullptr) {
      if (sprite.spriteId == ecs::SpriteId::PLAYER_SHIP) {
        renderer->drawTextureRegion(textureIt->second, SPRITE_SOURCE_X, SPRITE_SOURCE_Y, SPRITE_SOURCE_WIDTH,
                                    SPRITE_SOURCE_HEIGHT, static_cast<int>(transform.x), static_cast<int>(transform.y),
                                    static_cast<int>(sprite.width), static_cast<int>(sprite.height));
      } else {
        renderer->drawTextureEx(textureIt->second, static_cast<int>(transform.x), static_cast<int>(transform.y),
                                static_cast<int>(sprite.width), static_cast<int>(sprite.height), 0.0, false, false);
      }
    } else {
      constexpr std::uint8_t PLACEHOLDER_RED = 100;
      constexpr std::uint8_t PLACEHOLDER_GREEN = 150;
      constexpr std::uint8_t PLACEHOLDER_BLUE = 255;
      constexpr std::uint8_t ALPHA_OPAQUE = 255;
      Color color = {.r = PLACEHOLDER_RED, .g = PLACEHOLDER_GREEN, .b = PLACEHOLDER_BLUE, .a = ALPHA_OPAQUE};
      renderer->drawRect(static_cast<int>(transform.x), static_cast<int>(transform.y), static_cast<int>(sprite.width),
                         static_cast<int>(sprite.height), color);
    }
  }

  if (m_lobbyFont != nullptr) {
    std::string text = "Waiting for Players. Press X to start the game";

    int winWidth = renderer->getWindowWidth();
    int winHeight = renderer->getWindowHeight();

    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(m_lobbyFont, text, textWidth, textHeight);

    int textPosX = (winWidth - textWidth) / 2;
    int textPosY = (winHeight - textHeight) / 2;

    constexpr std::uint8_t TEXT_WHITE = 255;
    constexpr std::uint8_t TEXT_ALPHA = 255;
    Color textColor = {.r = TEXT_WHITE, .g = TEXT_WHITE, .b = TEXT_WHITE, .a = TEXT_ALPHA};
    renderer->drawText(m_lobbyFont, text, textPosX, textPosY, textColor);
  }
}

void LobbyRoomState::processInput() {}

void LobbyRoomState::cleanup()
{
  freeSpriteTextures();
  m_lobbyFont = nullptr;
}

void LobbyRoomState::loadSpriteTextures()
{
  if (renderer == nullptr) {
    return;
  }

  try {
    void *playerTexture = renderer->loadTexture("client/assets/r-typesheet1.gif");
    m_spriteTextures[static_cast<std::uint32_t>(ecs::SpriteId::PLAYER_SHIP)] = playerTexture;
  } catch (const std::exception &e) {
    std::cerr << "LobbyRoomState: Failed to load player texture: " << e.what() << '\n';
  }
}

void LobbyRoomState::freeSpriteTextures()
{
  if (renderer == nullptr) {
    return;
  }

  for (auto &[spriteId, texture] : m_spriteTextures) {
    if (texture != nullptr) {
      renderer->freeTexture(texture);
    }
  }
  m_spriteTextures.clear();
}
