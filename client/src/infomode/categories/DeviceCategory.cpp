/**
 * @file DeviceCategory.cpp
 * @brief Implementation of device information category
 */

#include "DeviceCategory.hpp"
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#else
#include <fstream>
#include <sstream>
#endif

DeviceCategory::DeviceCategory()
{
}

std::vector<std::string> DeviceCategory::getInfoLines() const
{
  std::vector<std::string> lines;

  // Operating System
  lines.push_back("OS: " + getOperatingSystem());

  // Graphics Information
  lines.push_back("Graphics: " + getGraphicsInfo());

  // Display Resolution
  lines.push_back("Display: " + getDisplayResolution());

  // Add more device info as needed
  lines.push_back("Architecture: N/A");
  lines.push_back("Storage: N/A");

  return lines;
}

void DeviceCategory::update(float deltaTime)
{
  // Device info doesn't change frequently, cache it
  if (!m_infoCached) {
    m_osInfo = getOperatingSystem();
    m_graphicsInfo = getGraphicsInfo();
    m_displayInfo = getDisplayResolution();
    m_infoCached = true;
  }
}

std::string DeviceCategory::getOperatingSystem() const
{
#ifdef _WIN32
  OSVERSIONINFOEX osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
    std::stringstream ss;
    ss << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
    return ss.str();
  }
  return "Windows (Unknown)";

#elif __APPLE__
  // macOS version detection
  return "macOS (Version detection TBD)";

#else
  // Linux distribution detection
  std::ifstream file("/etc/os-release");
  std::string line;
  std::string distro = "Linux";

  while (std::getline(file, line)) {
    if (line.find("PRETTY_NAME=") == 0) {
      distro = line.substr(13, line.size() - 14); // Remove quotes
      break;
    }
  }
  return distro;
#endif
}

std::string DeviceCategory::getGraphicsInfo() const
{
  // Placeholder - would need platform-specific graphics API detection
  return "Graphics detection TBD";
}

std::string DeviceCategory::getDisplayResolution() const
{
  // Placeholder - would need display enumeration
  return "Resolution detection TBD";
}
