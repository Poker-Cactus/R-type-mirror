/**
 * @file CpuRamCategory.hpp
 * @brief CPU and RAM information category
 */

#pragma once

#include "../InfoCategory.hpp"
#include <cstdint>
#include <memory>

#ifdef _WIN32
#include <psapi.h>
#include <windows.h>
#elif __APPLE__
#include <mach/host_priv.h>
#include <mach/kern_return.h>
#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/processor_info.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#else
#include <fstream>
#include <sstream>
#include <unistd.h>
#endif

/**
 * @class CpuRamCategory
 * @brief Displays CPU usage and RAM information
 */
class CpuRamCategory : public InfoCategory
{
public:
  /**
   * @brief Constructor
   */
  CpuRamCategory();

  /**
   * @brief Destructor
   */
  ~CpuRamCategory() override = default;

  /**
   * @brief Get category name
   * @return "CPU & RAM"
   */
  std::string getName() const override { return "CPU & RAM"; }

  /**
   * @brief Get information lines
   * @return Vector with CPU and RAM info
   */
  std::vector<std::string> getInfoLines() const override;

  /**
   * @brief Update CPU and RAM information
   */
  void update() override;

  /**
   * @brief Get the update interval for this category in frames (at 60 FPS)
   * @return 15 frames (every 0.25 seconds at 60 FPS)
   */
  int getUpdateIntervalFrames() const override { return 15; }

private:
  /**
   * @brief Get CPU usage percentage
   * @return CPU usage as percentage (0-100)
   */
  float getCpuUsage();

  /**
   * @brief Get number of CPU cores
   * @return Number of CPU cores
   */
  int getCpuCores() const;

  /**
   * @brief Get RAM usage information
   * @return Pair of used RAM in MB and total RAM in MB
   */
  std::pair<uint64_t, uint64_t> getRamUsage() const;

  // CPU monitoring state
  float m_lastCpuUsage = 0.0f;

  // RAM monitoring state
  uint64_t m_lastRamUsed = 0;
  uint64_t m_lastRamTotal = 0;
};
