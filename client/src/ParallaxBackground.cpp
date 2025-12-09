/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ParallaxBackground.cpp
*/

#include "ParallaxBackground.hpp"
#include <cmath>
#include <iostream>

ParallaxBackground::ParallaxBackground(IRenderer *renderer) : renderer(renderer), windowWidth(0), windowHeight(0) {}

ParallaxBackground::~ParallaxBackground()
{
    cleanup();
}

bool ParallaxBackground::init()
{
    if (!renderer) {
        return false;
    }

    // Récupérer les dimensions de la fenêtre
    windowWidth = renderer->getWindowWidth();
    windowHeight = renderer->getWindowHeight();

    // Ajouter les 3 couches de background avec des vitesses différentes
    // Couche 1 (arrière-plan lointain) - la plus lente
    addLayer("./client/assets/background/starfield.png", 20.0f, 1.0f);
    
    // Couche 2 (plan intermédiaire) - vitesse moyenne
    addLayer("./client/assets/background/diagonal.png", 50.0f, 1.0f);
    
    // Couche 3 (premier plan) - la plus rapide
    addLayer("./client/assets/background/basic.png", 100.0f, 1.0f);

    return !layers.empty();
}

void ParallaxBackground::addLayer(const std::string &texturePath, float scrollSpeed, float scale)
{
    if (!renderer) {
        return;
    }

    ParallaxLayer layer;
    layer.texture = renderer->loadTexture(texturePath);
    
    if (!layer.texture) {
        std::cerr << "Failed to load parallax layer: " << texturePath << std::endl;
        return;
    }

    layer.scrollSpeed = scrollSpeed;
    layer.scale = scale;
    layer.offsetX = 0.0f;

    layers.push_back(layer);
}

void ParallaxBackground::update(float dt)
{
    // Mettre à jour l'offset de chaque couche
    for (auto &layer : layers) {
        layer.offsetX += layer.scrollSpeed * dt;
        
        // Utiliser la largeur de l'écran comme référence pour la répétition
        // Réinitialiser l'offset pour créer l'effet infini
        if (layer.offsetX >= windowWidth) {
            layer.offsetX = std::fmod(layer.offsetX, static_cast<float>(windowWidth));
        }
    }
}

void ParallaxBackground::render()
{
    if (!renderer) {
        return;
    }

    // Dessiner chaque couche du fond vers le devant
    for (const auto &layer : layers) {
        renderLayer(layer);
    }
}

void ParallaxBackground::renderLayer(const ParallaxLayer &layer)
{
    if (!layer.texture) {
        return;
    }

    // On dessine la texture 3 fois côte à côte pour assurer la couverture complète
    // Position de départ (décalée par l'offset)
    int startX = -static_cast<int>(layer.offsetX);

    // Dessiner 3 copies de la texture pour couvrir l'écran + défilement
    for (int i = 0; i < 3; ++i) {
        int x = startX + (i * windowWidth);
        
        // Dessiner la texture en plein écran
        renderer->drawTextureEx(layer.texture, x, 0, windowWidth, windowHeight, 0.0, false, false);
    }
}

void ParallaxBackground::cleanup()
{
    if (!renderer) {
        return;
    }

    for (auto &layer : layers) {
        if (layer.texture) {
            renderer->freeTexture(layer.texture);
            layer.texture = nullptr;
        }
    }
    
    layers.clear();
}
