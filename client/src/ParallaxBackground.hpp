/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ParallaxBackground.hpp
*/

#pragma once
#include "../interface/IRenderer.hpp"
#include <string>
#include <vector>

/**
 * @brief Layer de parallaxe avec défilement infini
 */
struct ParallaxLayer
{
    void *texture = nullptr;      // Texture de la couche
    float scrollSpeed = 0.0f;     // Vitesse de défilement (plus élevée = plus rapide)
    float offsetX = 0.0f;         // Décalage horizontal actuel
    int offsetY = 0;              // Décalage vertical (position Y)
    float scale = 1.0f;           // Échelle de la texture
    int textureWidth = 0;         // Largeur native de la texture
    int textureHeight = 0;        // Hauteur native de la texture
};

/**
 * @brief Gère un background avec effet parallaxe infini
 * 
 * Cette classe gère plusieurs couches de background qui défilent
 * à des vitesses différentes pour créer un effet de profondeur.
 * Le défilement est infini grâce à une répétition seamless.
 */
class ParallaxBackground
{
  public:
    ParallaxBackground(IRenderer *renderer);
    ~ParallaxBackground();

    /**
     * @brief Initialise le background avec les textures
     * @return true si l'initialisation a réussi
     */
    bool init();

    /**
     * @brief Met à jour les positions des couches
     * @param dt Delta time en secondes
     */
    void update(float dt);

    /**
     * @brief Dessine toutes les couches du background
     */
    void render();

    /**
     * @brief Nettoie les ressources
     */
    void cleanup();

    /**
     * @brief Ajoute une couche de parallaxe
     * @param texturePath Chemin vers la texture
     * @param scrollSpeed Vitesse de défilement (pixels/seconde)
     * @param scale Échelle de la texture
     * @param offsetY Décalage vertical (position Y)
     */
    void addLayer(const std::string &texturePath, float scrollSpeed, float scale = 1.0f, int offsetY = 0);

  private:
    IRenderer *renderer;
    std::vector<ParallaxLayer> layers;
    int windowWidth = 0;
    int windowHeight = 0;

    /**
     * @brief Dessine une couche avec répétition pour l'effet infini
     * @param layer La couche à dessiner
     */
    void renderLayer(const ParallaxLayer &layer);
};
