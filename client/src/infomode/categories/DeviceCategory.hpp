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
   */
  void update() override;

  /**
   * @brief Get the update interval for this category in frames (at 60 FPS)
   * @return 600 frames (every 10 seconds at 60 FPS)
   */
  int getUpdateIntervalFrames() const override { return 600; }

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

  /**
   * @brief Get system architecture
   * @return CPU architecture
   */
  std::string getArchitecture() const;

  /**
   * @brief Get memory information
   * @return RAM size and usage
   */
  std::string getMemoryInfo() const;

  /**
   * @brief Get storage information
   * @return Disk space information
   */
  std::string getStorageInfo() const;

  // Cached information (updated infrequently)
  mutable std::string m_osInfo;
  mutable std::string m_architectureInfo;
  mutable std::string m_graphicsInfo;
  mutable std::string m_displayInfo;
  mutable std::string m_memoryInfo;
  mutable std::string m_storageInfo;
  mutable bool m_infoCached = false;
};
