/**
 * @file ParallaxBackground.hpp
 * @brief Parallax scrolling background system
 */

#pragma once
#include "../interface/Color.hpp"
#include "../interface/IRenderer.hpp"
#include <string>
#include <vector>

/**
 * @struct Star
 * @brief Represents a procedural star in the background
 */
struct Star {
  float x = 0.0f;     ///< X position
  float y = 0.0f;     ///< Y position
  float radius = 1.0f; ///< Star radius
  Color color;        ///< Star color
};

/**
 * @struct ParallaxLayer
 * @brief Background layer with infinite scrolling
 */
struct ParallaxLayer {
  void *texture = nullptr;      ///< Layer texture
  float scrollSpeed = 0.0f;     ///< Scroll speed in pixels/second
  float offsetX = 0.0f;         ///< Current horizontal offset
  int offsetY = 0;              ///< Vertical offset
  float scale = 1.0f;           ///< Texture scale
  int textureWidth = 0;         ///< Texture width
  int textureHeight = 0;        ///< Texture height
  std::vector<Star> stars;      ///< Procedural stars (if used)
  bool isProcedural = false;    ///< Whether layer is procedurally generated
};

/**
 * @class ParallaxBackground
 * @brief Manages multi-layer parallax scrolling background
 *
 * Handles multiple background layers scrolling at different speeds
 * to create depth. Supports both texture-based and procedural star layers
 * with infinite seamless scrolling.
 */
class ParallaxBackground
{
public:
  // R-Type inspired high-speed parallax configuration
  static constexpr int SLOW_STAR_COUNT = 120;       ///< Slow star layer count
  static constexpr float SLOW_SPEED = 100.0F;       ///< Slow layer speed
  static constexpr float SLOW_MIN_RADIUS = 1.5F;    ///< Slow star min radius
  static constexpr float SLOW_MAX_RADIUS = 2.5F;    ///< Slow star max radius

  static constexpr int MEDIUM_STAR_COUNT = 80;      ///< Medium star layer count
  static constexpr float MEDIUM_SPEED = 280.0F;     ///< Medium layer speed
  static constexpr float MEDIUM_MIN_RADIUS = 2.0F;  ///< Medium star min radius
  static constexpr float MEDIUM_MAX_RADIUS = 3.5F;  ///< Medium star max radius

  static constexpr int FAST_STAR_COUNT = 50;        ///< Fast star layer count
  static constexpr float FAST_SPEED = 550.0F;       ///< Fast layer speed
  static constexpr float FAST_MIN_RADIUS = 2.5F;    ///< Fast star min radius
  static constexpr float FAST_MAX_RADIUS = 4.5F;    ///< Fast star max radius

  /**
   * @brief Construct parallax background
   * @param renderer Renderer interface
   */
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
