/**
 * @file InfoCategory.hpp
 * @brief Base class for info mode categories
 */

#pragma once

#include <string>
#include <vector>

/**
 * @class InfoCategory
 * @brief Abstract base class for different info categories
 *
 * This class defines the interface that all info categories must implement.
 * Each category is responsible for gathering and formatting its own information.
 */
class InfoCategory
{
public:
  /**
   * @brief Virtual destructor
   */
  virtual ~InfoCategory() = default;

  /**
   * @brief Get the display name of this category
   * @return Category name as string
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Get the information lines to display
   * @return Vector of strings, each representing a line of info
   */
  virtual std::vector<std::string> getInfoLines() const = 0;

  /**
   * @brief Update the category's information
   */
  virtual void update() = 0;

  /**
   * @brief Get the update interval for this category in frames (at 60 FPS)
   * @return Number of frames between updates (e.g., 120 for every 2 seconds at 60 FPS)
   */
  virtual int getUpdateIntervalFrames() const = 0;
};
