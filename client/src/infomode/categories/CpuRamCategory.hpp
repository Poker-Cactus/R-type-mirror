/**
 * @file CpuRamCategory.hpp
 * @brief CPU and RAM information category
 */

#pragma once

#include "../InfoCategory.hpp"
#include <memory>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif __APPLE__
#include <sys/sysctl.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/kern_return.h>
#include <mach/host_priv.h>
#include <mach/processor_info.h>
#else
#include <unistd.h>
#include <fstream>
#include <sstream>
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
   * @param deltaTime Time elapsed since last update
   */
  void update(float deltaTime) override;

  /**
   * @brief This category needs frequent updates for accurate CPU readings
   * @return true
   */
  bool needsFrequentUpdates() const override { return true; }

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
  int m_sampleCount = 0;
  static constexpr int SAMPLES_PER_UPDATE = 60; // Update every ~1 second at 60 FPS
};
