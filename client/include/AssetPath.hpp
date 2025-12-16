/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AssetPath - Helper to resolve asset paths across different deployment scenarios
*/

#pragma once

#include <string>
#include <sys/stat.h>

inline bool fileExists(const std::string &path)
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

inline std::string resolveAssetPath(const std::string &relativePath)
{
  // Remove leading "./" if present
  std::string cleanPath = relativePath;
  if (cleanPath.substr(0, 2) == "./") {
    cleanPath = cleanPath.substr(2);
  }
  
  // List of possible base paths to try
  const std::string basePaths[] = {
    "",                    // Current directory (for portable package)
    "./",                  // Explicit current directory
    "../",                 // Parent directory
    "./client/",          // Build directory structure
    "../client/"          // Parent build structure
  };
  
  for (const auto &basePath : basePaths) {
    std::string fullPath = basePath + cleanPath;
    if (fileExists(fullPath)) {
      return fullPath;
    }
  }
  
  // If nothing found, return the original path
  return relativePath;
}
