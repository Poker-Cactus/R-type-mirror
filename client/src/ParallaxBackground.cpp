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

  // Responsive
  addLayer("./client/assets/background/iceberg/0-water.png", 50.0f, 1.0f, 0);
  addLayer("./client/assets/background/iceberg/1-Sky.png", 5.0f, 1.0f, -300);
  addLayer("./client/assets/background/iceberg/2-cloud.png", 10.0f, 1.0f, -150);
  addLayer("./client/assets/background/iceberg/3-mountains.png", 40.0f, 0.02f, -240);
  addLayer("./client/assets/background/iceberg/2-2-water reflex back.png", 250.0f, 1.0f, -200);
  addLayer("./client/assets/background/iceberg/2-1-water reflex.png", 800.0f, 1.0f, -200);
  addLayer("./client/assets/background/iceberg/4-icebergreflex.png", 220.0f, 1.0f, 120);
  addLayer("./client/assets/background/iceberg/5-iceberg.png", 220.0f, 1.0f, 120);

  return !layers.empty();
}

void ParallaxBackground::addLayer(const std::string &texturePath, float scrollSpeed, float scale, int offsetY)
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

  // Récupérer les dimensions réelles de la texture via l'interface
  renderer->getTextureSize(layer.texture, layer.textureWidth, layer.textureHeight);

  layer.scrollSpeed = scrollSpeed;
  layer.scale = scale;
  layer.offsetX = 0.0f;
  layer.offsetY = offsetY;

  layers.push_back(layer);
}

void ParallaxBackground::update(float dt)
{
  // Mettre à jour l'offset de chaque couche
  for (auto &layer : layers) {
    layer.offsetX += layer.scrollSpeed * dt;

    // Utiliser la largeur réelle de la texture pour la répétition seamless
    // Réinitialiser l'offset quand on a dépassé une largeur de texture
    if (layer.offsetX >= layer.textureWidth) {
      layer.offsetX = std::fmod(layer.offsetX, static_cast<float>(layer.textureWidth));
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

  // Position de départ (décalée par l'offset)
  int startX = -static_cast<int>(layer.offsetX);

  // Dessiner plusieurs copies de la texture pour couvrir l'écran
  // Utiliser la largeur réelle de la texture pour les mettre bout à bout sans overlap
  for (int i = 0; i < 5; ++i) {
    int x = startX + (i * layer.textureWidth);
    renderer->drawTexture(layer.texture, x, layer.offsetY);
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
