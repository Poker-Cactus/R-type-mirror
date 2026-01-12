/**
 * @file AssetPath.hpp
 * @brief Asset path resolution utilities
 *
 * Provides helper functions to resolve asset paths across different
 * deployment scenarios (development, build directory, portable package).
 */

#pragma once

#include <string>
#include <sys/stat.h>

/**
 * @brief Check if a file exists at the given path
 * @param path File path to check
 * @return true if file exists, false otherwise
 */
inline bool fileExists(const std::string &path)
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

/**
 * @brief Resolve relative asset path to absolute path
 *
 * Attempts to locate the asset file by searching multiple possible
 * base directories. This handles different deployment scenarios:
 * - Development builds in build directory
 * - Portable packages with assets in root
 * - Assets with or without 'client/' prefix
 *
 * @param relativePath Relative path to the asset
 * @return Resolved absolute path if found, original path otherwise
 */
inline std::string resolveAssetPath(const std::string &relativePath)
{
  // Remove leading "./" if present
  std::string cleanPath = relativePath;
  if (cleanPath.substr(0, 2) == "./") {
    cleanPath = cleanPath.substr(2);
  }

  // List of possible base paths to try
  const std::string basePaths[] = {
    "", // Current directory (for portable package)
    "./", // Explicit current directory
    "../", // Parent directory
    "./client/", // Build directory structure
    "../client/" // Parent build structure
  };

  for (const auto &basePath : basePaths) {
    std::string fullPath = basePath + cleanPath;
    if (fileExists(fullPath)) {
      return fullPath;
    }
  }

  // Try stripping "client/" prefix for portable packages
  // In portable packages, assets are at "assets/..." not "client/assets/..."
  if (cleanPath.substr(0, 7) == "client/") {
    std::string withoutClientPrefix = cleanPath.substr(7);
    for (const auto &basePath : basePaths) {
      std::string fullPath = basePath + withoutClientPrefix;
      if (fileExists(fullPath)) {
        return fullPath;
      }
    }
  }

  // If nothing found, return the original path
  return relativePath;
}
