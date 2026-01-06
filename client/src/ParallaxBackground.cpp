/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ParallaxBackground.cpp
*/

#include "ParallaxBackground.hpp"
#include "../include/AssetPath.hpp"
#include "../interface/Geometry.hpp"
#include <cmath>
#include <iostream>
#include <random>

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

  windowWidth = renderer->getWindowWidth();
  windowHeight = renderer->getWindowHeight();

  addStarLayerWithVariedColors(SLOW_STAR_COUNT, SLOW_SPEED, SLOW_MIN_RADIUS, SLOW_MAX_RADIUS);
  addStarLayerWithVariedColors(MEDIUM_STAR_COUNT, MEDIUM_SPEED, MEDIUM_MIN_RADIUS, MEDIUM_MAX_RADIUS);
  addStarLayerWithVariedColors(FAST_STAR_COUNT, FAST_SPEED, FAST_MIN_RADIUS, FAST_MAX_RADIUS);

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

  renderer->getTextureSize(layer.texture, layer.textureWidth, layer.textureHeight);

  layer.scrollSpeed = scrollSpeed;
  layer.scale = scale;
  layer.offsetX = 0.0f;
  layer.offsetY = offsetY;

  layers.push_back(layer);
}

void ParallaxBackground::addStarLayer(
  int starCount, float scrollSpeed, float minRadius, float maxRadius, const Color &color)
{
  if (!renderer) {
    return;
  }

  ParallaxLayer layer;
  layer.texture = nullptr;
  layer.scrollSpeed = scrollSpeed;
  layer.scale = 1.0f;
  layer.offsetX = 0.0f;
  layer.offsetY = 0;
  layer.isProcedural = true;
  layer.textureWidth = windowWidth * 2;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(layer.textureWidth));
  std::uniform_real_distribution<float> yDist(0.0f, static_cast<float>(windowHeight));
  std::uniform_real_distribution<float> radiusDist(minRadius, maxRadius);

  layer.stars.reserve(starCount);
  for (int i = 0; i < starCount; ++i) {
    Star star;
    star.x = xDist(gen);
    star.y = yDist(gen);
    star.radius = radiusDist(gen);
    star.color = color;
    layer.stars.push_back(star);
  }

  layers.push_back(layer);
}

void ParallaxBackground::addStarLayerWithVariedColors(
  int starCount, float scrollSpeed, float minRadius, float maxRadius)
{
  if (!renderer) {
    return;
  }

  ParallaxLayer layer;
  layer.texture = nullptr;
  layer.scrollSpeed = scrollSpeed;
  layer.scale = 1.0f;
  layer.offsetX = 0.0f;
  layer.offsetY = 0;
  layer.isProcedural = true;
  layer.textureWidth = windowWidth * 2;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(layer.textureWidth));
  std::uniform_real_distribution<float> yDist(0.0f, static_cast<float>(windowHeight));
  std::uniform_real_distribution<float> radiusDist(minRadius, maxRadius);
  std::uniform_real_distribution<float> colorDist(0.0f, 100.0f);

  layer.stars.reserve(starCount);
  for (int i = 0; i < starCount; ++i) {
    Star star;
    star.x = xDist(gen);
    star.y = yDist(gen);
    star.radius = radiusDist(gen);
    
    // Color distribution: dark blue, blue-green, light blue, white, purple
    float colorChoice = colorDist(gen);
    if (colorChoice < 35.0f) {
      // 35%: Dark blue
      star.color = {25, 25, 112, 255};  // Midnight blue
    } else if (colorChoice < 60.0f) {
      // 25%: Blue-green (cyan tones)
      std::uniform_int_distribution<int> cyanDist(0, 2);
      int cyanVariant = cyanDist(gen);
      if (cyanVariant == 0) {
        star.color = {0, 139, 139, 240};    // Dark cyan
      } else if (cyanVariant == 1) {
        star.color = {32, 178, 170, 245};   // Light sea green
      } else {
        star.color = {64, 224, 208, 250};   // Turquoise
      }
    } else if (colorChoice < 80.0f) {
      // 20%: Light blue
      std::uniform_int_distribution<int> lightBlueDist(0, 1);
      if (lightBlueDist(gen) == 0) {
        star.color = {135, 206, 250, 255};  // Sky blue
      } else {
        star.color = {173, 216, 230, 255};  // Light blue
      }
    } else if (colorChoice < 90.0f) {
      // 10%: White
      star.color = {255, 255, 255, 255};
    } else {
      // 10%: Purple/Violet
      std::uniform_int_distribution<int> purpleDist(0, 2);
      int purpleVariant = purpleDist(gen);
      if (purpleVariant == 0) {
        star.color = {138, 43, 226, 250};   // Blue violet
      } else if (purpleVariant == 1) {
        star.color = {148, 0, 211, 245};    // Dark violet
      } else {
        star.color = {186, 85, 211, 240};   // Medium orchid
      }
    }
    
    layer.stars.push_back(star);
  }

  layers.push_back(layer);
}

void ParallaxBackground::update(float dt)
{
  for (auto &layer : layers) {
    layer.offsetX += layer.scrollSpeed * dt;

    const float layerWidth = static_cast<float>(layer.textureWidth);
    if (layer.offsetX >= layerWidth) {
      layer.offsetX = std::fmod(layer.offsetX, layerWidth);
    }
  }
}

void ParallaxBackground::render()
{
  if (!renderer) {
    return;
  }

  for (const auto &layer : layers) {
    renderLayer(layer);
  }
}

void ParallaxBackground::renderLayer(const ParallaxLayer &layer)
{
  if (layer.isProcedural) {
    const float layerWidth = static_cast<float>(layer.textureWidth);
    
    for (const auto &star : layer.stars) {
      // Calculate position with offset, using modulo for seamless wrapping
      float baseX = star.x - layer.offsetX;
      
      // Normalize position to layerWidth range
      baseX = std::fmod(baseX, layerWidth);
      if (baseX < 0.0f) {
        baseX += layerWidth;
      }
      
      // Draw star at its primary position
      if (baseX >= 0.0f && baseX <= static_cast<float>(windowWidth)) {
        Circle circle;
        circle.centerX = static_cast<int>(baseX);
        circle.centerY = static_cast<int>(star.y);
        circle.radius = static_cast<int>(star.radius);
        renderer->drawCircleFilled(circle, star.color);
      }
      
      // Draw wrapped copy to ensure continuity at screen edges
      float wrappedX = baseX - layerWidth;
      if (wrappedX >= -10.0f && wrappedX <= static_cast<float>(windowWidth)) {
        Circle circle;
        circle.centerX = static_cast<int>(wrappedX);
        circle.centerY = static_cast<int>(star.y);
        circle.radius = static_cast<int>(star.radius);
        renderer->drawCircleFilled(circle, star.color);
      }
      
      // Draw another wrapped copy on the right side
      wrappedX = baseX + layerWidth;
      if (wrappedX >= 0.0f && wrappedX <= static_cast<float>(windowWidth + 10)) {
        Circle circle;
        circle.centerX = static_cast<int>(wrappedX);
        circle.centerY = static_cast<int>(star.y);
        circle.radius = static_cast<int>(star.radius);
        renderer->drawCircleFilled(circle, star.color);
      }
    }
  } else if (layer.texture) {
    int startX = -static_cast<int>(layer.offsetX);
    for (int i = 0; i < 5; ++i) {
      int x = startX + (i * layer.textureWidth);
      renderer->drawTexture(layer.texture, x, layer.offsetY);
    }
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
