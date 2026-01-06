/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ParallaxBackground.hpp
*/

#pragma once
#include "../interface/IRenderer.hpp"
#include "../interface/Color.hpp"
#include <string>
#include <vector>

/**
 * @brief Structure representing a star
 */
struct Star {
  float x = 0.0f;
  float y = 0.0f;
  float radius = 1.0f;
  Color color;
};

/**
 * @brief Layer structure for parallax with infinite scrolling
 */
struct ParallaxLayer {
  void *texture = nullptr;
  float scrollSpeed = 0.0f;
  float offsetX = 0.0f;
  int offsetY = 0;
  float scale = 1.0f;
  int textureWidth = 0;
  int textureHeight = 0;
  std::vector<Star> stars;
  bool isProcedural = false;
};

/**
 * @brief Manages a parallax background with infinite scrolling effect
 *
 * This class handles multiple background layers that scroll at different speeds
 * to create a depth effect. Scrolling is infinite thanks to seamless repetition.
 */
class ParallaxBackground
{
public:
  // Star layer configuration constants - R-Type inspired high-speed parallax
  static constexpr int SLOW_STAR_COUNT = 120;
  static constexpr float SLOW_SPEED = 100.0F;
  static constexpr float SLOW_MIN_RADIUS = 1.5F;
  static constexpr float SLOW_MAX_RADIUS = 2.5F;

  static constexpr int MEDIUM_STAR_COUNT = 80;
  static constexpr float MEDIUM_SPEED = 280.0F;
  static constexpr float MEDIUM_MIN_RADIUS = 2.0F;
  static constexpr float MEDIUM_MAX_RADIUS = 3.5F;

  static constexpr int FAST_STAR_COUNT = 50;
  static constexpr float FAST_SPEED = 550.0F;
  static constexpr float FAST_MIN_RADIUS = 2.5F;
  static constexpr float FAST_MAX_RADIUS = 4.5F;

  ParallaxBackground(IRenderer *renderer);
  ~ParallaxBackground();

  /**
   * @brief Initialize background with textures
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Update layer positions
   * @param dt Delta time in seconds
   */
  void update(float dt);

  /**
   * @brief Render all background layers
   */
  void render();

  /**
   * @brief Clean up resources
   */
  void cleanup();

  /**
   * @brief Add a parallax layer
   * @param texturePath Texture file path
   * @param scrollSpeed Scroll speed (pixels/second)
   * @param scale Texture scale
   * @param offsetY Vertical offset (Y position)
   */
  void addLayer(const std::string &texturePath, float scrollSpeed, float scale = 1.0f, int offsetY = 0);

  /**
   * @brief Add a procedural star layer
   * @param starCount Number of stars to generate
   * @param scrollSpeed Scroll speed (pixels/second)
   * @param minRadius Minimum star radius
   * @param maxRadius Maximum star radius
   * @param color Star color
   */
  void addStarLayer(int starCount, float scrollSpeed, float minRadius, float maxRadius, const Color &color);

  /**
   * @brief Add a procedural star layer with varied colors
   * @param starCount Number of stars to generate
   * @param scrollSpeed Scroll speed (pixels/second)
   * @param minRadius Minimum star radius
   * @param maxRadius Maximum star radius
   * Colors: 80% blue (various shades), 10% white, 10% light purple
   */
  void addStarLayerWithVariedColors(int starCount, float scrollSpeed, float minRadius, float maxRadius);

private:
  IRenderer *renderer;
  std::vector<ParallaxLayer> layers;
  int windowWidth = 0;
  int windowHeight = 0;

  /**
   * @brief Render a layer with repetition for infinite effect
   * @param layer The layer to render
   */
  void renderLayer(const ParallaxLayer &layer);
};
