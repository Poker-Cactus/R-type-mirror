/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** MapCollision.hpp - Component for map/tilemap collision data from TMX files
*/

#ifndef ENGINECORE_ECS_COMPONENTS_MAPCOLLISION_HPP
#define ENGINECORE_ECS_COMPONENTS_MAPCOLLISION_HPP

#include "IComponent.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ecs
{

/**
 * @brief MapCollision component for storing tilemap collision data from TMX files
 * Parses Tiled TMX format and extracts collision layer data
 */
struct MapCollision : public IComponent {
  int tileWidth = 8;   // Width of each tile in pixels
  int tileHeight = 8;  // Height of each tile in pixels
  int mapWidth = 0;    // Map width in tiles
  int mapHeight = 0;   // Map height in tiles
  std::vector<int> collisionData; // Tile IDs from collision layer (0 = no collision, non-zero = collision)

  MapCollision() = default;

  /**
   * @brief Load collision data from a TMX file
   * Extracts the layer named "ruins" for collision detection
   * @param filepath Path to the TMX file
   * @return true if loaded successfully
   */
  bool loadFromFile(const std::string &filepath)
  {
    try {
      std::ifstream file(filepath);
      if (!file.is_open()) {
        return false;
      }

      std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      file.close();

      // Parse map dimensions and tile size
      mapWidth = extractIntAttribute(content, "<map", "width");
      mapHeight = extractIntAttribute(content, "<map", "height");
      tileWidth = extractIntAttribute(content, "<map", "tilewidth");
      tileHeight = extractIntAttribute(content, "<map", "tileheight");

      // Find the "ruins" collision layer
      size_t layerPos = content.find("<layer id=\"1\" name=\"ruins\"");
      if (layerPos == std::string::npos) {
        return false;
      }

      // Find the CSV data section within the layer
      size_t dataStartPos = content.find("<data encoding=\"csv\">", layerPos);
      if (dataStartPos == std::string::npos) {
        return false;
      }
      dataStartPos += 21; // Skip "<data encoding=\"csv\">"

      size_t dataEndPos = content.find("</data>", dataStartPos);
      if (dataEndPos == std::string::npos) {
        return false;
      }

      std::string csvData = content.substr(dataStartPos, dataEndPos - dataStartPos);

      // Parse CSV tile data
      collisionData.clear();
      std::istringstream stream(csvData);
      std::string token;
      while (std::getline(stream, token, ',')) {
        // Remove whitespace and newlines
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty()) {
          collisionData.push_back(std::stoi(token));
        }
      }

      return true;
    } catch (...) {
      return false;
    }
  }

  /**
   * @brief Check if a point collides with the tilemap
   * @param x X position in world coordinates (pixels)
   * @param y Y position in world coordinates (pixels)
   * @return true if there is a collision
   */
  bool checkCollision(float x, float y) const
  {
    if (collisionData.empty() || tileWidth == 0 || tileHeight == 0) {
      return false;
    }

    int tileX = static_cast<int>(x) / tileWidth;
    int tileY = static_cast<int>(y) / tileHeight;

    // Check bounds
    if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
      return false;
    }

    int index = tileY * mapWidth + tileX;
    if (index >= 0 && index < static_cast<int>(collisionData.size())) {
      // Non-zero tile ID means collision
      return collisionData[index] != 0;
    }
    return false;
  }

  /**
   * @brief Check if a rectangle collides with the tilemap
   * @param x X position of top-left corner (pixels)
   * @param y Y position of top-left corner (pixels)
   * @param width Width of the rectangle (pixels)
   * @param height Height of the rectangle (pixels)
   * @return true if there is a collision
   */
  bool checkRectCollision(float x, float y, float width, float height) const
  {
    // Check corners and edges of the rectangle
    return checkCollision(x, y) ||                         // Top-left
           checkCollision(x + width - 1, y) ||             // Top-right
           checkCollision(x, y + height - 1) ||            // Bottom-left
           checkCollision(x + width - 1, y + height - 1) || // Bottom-right
           checkCollision(x + width / 2, y + height / 2);   // Center
  }

  [[nodiscard]] nlohmann::json toJson() const override
  {
    nlohmann::json json;
    json["tileWidth"] = tileWidth;
    json["tileHeight"] = tileHeight;
    json["mapWidth"] = mapWidth;
    json["mapHeight"] = mapHeight;
    json["tileCount"] = collisionData.size();
    return json;
  }

private:
  /**
   * @brief Extract integer attribute from XML tag
   */
  int extractIntAttribute(const std::string &content, const std::string &tagStart, const std::string &attribute) const
  {
    size_t tagPos = content.find(tagStart);
    if (tagPos == std::string::npos) {
      return 0;
    }

    size_t tagEnd = content.find(">", tagPos);
    std::string tag = content.substr(tagPos, tagEnd - tagPos);

    size_t attrPos = tag.find(attribute + "=\"");
    if (attrPos == std::string::npos) {
      return 0;
    }

    size_t valueStart = attrPos + attribute.length() + 2;
    size_t valueEnd = tag.find("\"", valueStart);
    std::string value = tag.substr(valueStart, valueEnd - valueStart);

    return std::stoi(value);
  }
};

} // namespace ecs

#endif // ENGINECORE_ECS_COMPONENTS_MAPCOLLISION_HPP
