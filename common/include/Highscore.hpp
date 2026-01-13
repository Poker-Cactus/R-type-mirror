/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Highscore.hpp - Highscore management system
*/

#ifndef HIGHSCORE_HPP_
#define HIGHSCORE_HPP_

#include "Common.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/**
 * @struct HighscoreEntry
 * @brief Represents a single highscore entry
 */
struct HighscoreEntry {
  std::string username; ///< Player username
  int score = 0; ///< Final score achieved
  Difficulty difficulty; ///< Game difficulty level

  // For JSON serialization
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(HighscoreEntry, username, score, difficulty)
};

/**
 * @class HighscoreManager
 * @brief Manages loading, saving, and updating highscores
 */
class HighscoreManager
{
public:
  HighscoreManager();
  ~HighscoreManager() = default;

  /**
   * @brief Load highscores from file
   * @return true if loading was successful
   */
  bool loadFromFile();

  /**
   * @brief Save highscores to file
   * @return true if saving was successful
   */
  bool saveToFile() const;

  /**
   * @brief Add a new highscore entry
   * @param entry The highscore entry to add
   * @return true if the entry was added to the highscores (top 5)
   */
  bool addHighscore(const HighscoreEntry &entry);

  /**
   * @brief Get the current highscores (top 5)
   * @return Vector of highscore entries, sorted by score descending
   */
  [[nodiscard]] const std::vector<HighscoreEntry> &getHighscores() const { return m_highscores; }

  /**
   * @brief Check if a score qualifies for the highscore list
   * @param score The score to check
   * @return true if the score would make it to the top 5
   */
  [[nodiscard]] bool isHighscore(int score) const;

  /**
   * @brief Refresh highscores from file (reload)
   * @return true if refresh was successful
   */
  bool refreshHighscores() { return loadFromFile(); }

  /**
   * @brief Clear all highscores
   */
  void clearHighscores();

private:
  static constexpr std::size_t MAX_HIGHSCORES = 5; ///< Maximum number of highscores to keep
  static constexpr const char *HIGHSCORE_FILE = "highscores.json"; ///< Filename for highscore storage

  std::vector<HighscoreEntry> m_highscores; ///< Current highscore list

  /**
   * @brief Sort highscores by score in descending order
   */
  void sortHighscores();

  /**
   * @brief Get the file path for highscore storage
   * @return Path to the highscore file
   */
  [[nodiscard]] fs::path getHighscoreFilePath() const;
};

#endif // HIGHSCORE_HPP_
