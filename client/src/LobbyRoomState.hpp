/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyRoomState.hpp
*/

#pragma once
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../interface/IRenderer.hpp"
#include "Overlay.hpp"
#include "ParallaxBackground.hpp"
#include <memory>
#include <unordered_map>

namespace ecs
{
class World;
}

/**
 * @brief Manages the lobby waiting room before game starts
 */
class LobbyRoomState
{
public:
  LobbyRoomState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world);
  ~LobbyRoomState();

  bool init();
  void update(float deltaTime);
  void render();
  void processInput();
  void cleanup();

  [[nodiscard]] bool shouldStartGame() const { return m_startGameRequested; }

private:
  void loadSpriteTextures();
  void freeSpriteTextures();

  IRenderer *renderer;
  std::shared_ptr<ecs::World> world;
  std::unique_ptr<ParallaxBackground> background;
  std::unique_ptr<Overlay> overlay;
  std::unordered_map<std::uint32_t, void *> m_spriteTextures;
  void *m_lobbyFont = nullptr;
  bool m_startGameRequested = false;
};
