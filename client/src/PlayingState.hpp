/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayingState.hpp
*/

#pragma once
#include "../../engineCore/include/ecs/World.hpp"
#include "../interface/IRenderer.hpp"
#include "ParallaxBackground.hpp"
#include <memory>
#include <unordered_map>

/**
 * @brief Gère l'état du jeu en cours (gameplay)
 *
 * Cette classe gère tous les aspects du jeu actif, incluant:
 * - Le background parallaxe animé
 * - Les entités de jeu (joueur, ennemis, projectiles)
 * - La logique de gameplay
 */
class PlayingState
{
public:
  PlayingState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world);
  ~PlayingState();

  /**
   * @brief Initialise l'état de jeu
   * @return true si l'initialisation a réussi
   */
  bool init();

  /**
   * @brief Met à jour la logique du jeu
   * @param deltaTime Delta time en secondes
   */
  void update(float delta_time);

  /**
   * @brief Dessine tous les éléments du jeu
   */
  void render();

  /**
   * @brief Gère les entrées utilisateur pendant le jeu
   */
  void processInput();

  /**
   * @brief Nettoie les ressources
   */
  void cleanup();

  /**
   * @brief Check if player is dead and should return to menu
   */
  bool shouldReturnToMenu() const { return m_playerHealth <= 0; }

private:
  IRenderer *renderer;
  std::shared_ptr<ecs::World> world;
  std::unique_ptr<ParallaxBackground> background;

  // Texture management for sprite rendering
  std::unordered_map<std::uint32_t, void *> m_spriteTextures;

  void loadSpriteTextures();
  void freeSpriteTextures();

  // Animation system
  void updateAnimations(float deltaTime);

  // HUD state
  static constexpr int INITIAL_PLAYER_HEALTH = 100;
  void *m_hudFont = nullptr;
  int m_playerHealth = INITIAL_PLAYER_HEALTH;
  int m_playerScore = 0;

  void renderHUD();
  void updateHUDFromWorld();
};
