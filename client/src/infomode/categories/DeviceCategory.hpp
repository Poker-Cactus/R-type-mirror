/**
 * @file DeviceCategory.hpp
 * @brief Device information category
 */

#pragma once

#include "../InfoCategory.hpp"

/**
 * @class DeviceCategory
 * @brief Displays device and system information
 */
class DeviceCategory : public InfoCategory
{
public:
  /**
   * @brief Constructor
   */
  DeviceCategory();

  /**
   * @brief Destructor
   */
  ~DeviceCategory() override = default;

  /**
   * @brief Get category name
   * @return "Device"
   */
  std::string getName() const override { return "Device"; }

  /**
   * @brief Get information lines
   * @return Vector with device info
   */
  std::vector<std::string> getInfoLines() const override;

  /**
   * @brief Update device information
   * @param deltaTime Time elapsed since last update
   */
  void update(float deltaTime) override;

private:
  /**
   * @brief Get operating system information
   * @return OS name and version
   */
  std::string getOperatingSystem() const;

  /**
   * @brief Get graphics information
   * @return Graphics card and driver info
   */
  std::string getGraphicsInfo() const;

  /**
   * @brief Get display resolution
   * @return Screen resolution
   */
  std::string getDisplayResolution() const;

  // Cached information (updated infrequently)
  mutable std::string m_osInfo;
  mutable std::string m_graphicsInfo;
  mutable std::string m_displayInfo;
  mutable bool m_infoCached = false;
};
