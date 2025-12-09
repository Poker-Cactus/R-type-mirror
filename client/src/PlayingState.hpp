/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayingState.hpp
*/

#pragma once
#include "../interface/IRenderer.hpp"
#include "ParallaxBackground.hpp"
#include <memory>

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
    PlayingState(IRenderer *renderer);
    ~PlayingState();

    /**
     * @brief Initialise l'état de jeu
     * @return true si l'initialisation a réussi
     */
    bool init();

    /**
     * @brief Met à jour la logique du jeu
     * @param dt Delta time en secondes
     */
    void update(float dt);

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

  private:
    IRenderer *renderer;
    std::unique_ptr<ParallaxBackground> background;
    
    // TODO: Ajouter les systèmes de jeu
    // - Gestion des entités (ECS)
    // - Système de collision
    // - Système de spawn d'ennemis
    // - Interface de jeu (HUD)
};
