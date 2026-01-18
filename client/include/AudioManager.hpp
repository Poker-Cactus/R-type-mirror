/**
 * @file AudioManager.hpp
 * @brief Centralized audio management system
 */

#pragma once
#include "../interface/IRenderer.hpp"
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @enum AudioCategory
 * @brief Categories of audio for volume control
 */
enum class AudioCategory {
  MUSIC, ///< Background music
  SFX, ///< Sound effects
  UI ///< User interface sounds
};

/**
 * @class AudioManager
 * @brief Centralized manager for all game audio (music and sound effects)
 *
 * This class provides a single point of control for loading, playing,
 * and managing all audio resources in the game. It supports:
 * - Separate volume control for music, SFX, and UI sounds
 * - Named audio resources for easy reference
 * - Automatic resource cleanup
 * - Music transitions and crossfading
 */
class AudioManager
{
public:
  /**
   * @brief Construct the audio manager
   * @param renderer Renderer interface that provides audio capabilities
   */
  explicit AudioManager(std::shared_ptr<IRenderer> renderer);
  ~AudioManager();

  /**
   * @brief Initialize and load all audio resources
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Clean up all audio resources
   */
  void cleanup();

  /**
   * @brief Play a sound effect by name
   * @param soundName Name of the sound to play
   * @param loops Number of times to loop (-1 for infinite, 0 for once)
   */
  void playSound(const std::string &soundName, int loops = 0);

  /**
   * @brief Play music by name
   * @param musicName Name of the music to play
   * @param fadeOut Whether to fade out current music before playing new one
   */
  void playMusic(const std::string &musicName, bool fadeOut = false);

  /**
   * @brief Stop current music
   */
  void stopMusic();

  /**
   * @brief Pause current music
   */
  void pauseMusic();

  /**
   * @brief Resume paused music
   */
  void resumeMusic();

  /**
   * @brief Set master volume (affects all audio)
   * @param volume Volume level (0-100)
   */
  void setMasterVolume(int volume);

  /**
   * @brief Set music volume
   * @param volume Volume level (0-100)
   */
  void setMusicVolume(int volume);

  /**
   * @brief Set sound effects volume
   * @param volume Volume level (0-100)
   */
  void setSfxVolume(int volume);

  /**
   * @brief Check if a sound is loaded
   * @param soundName Name of the sound
   * @return true if the sound is loaded
   */
  bool isSoundLoaded(const std::string &soundName) const;

  /**
   * @brief Check if a music track is loaded
   * @param musicName Name of the music
   * @return true if the music is loaded
   */
  bool isMusicLoaded(const std::string &musicName) const;

  /**
   * @brief Get current music name
   * @return Name of currently playing music, empty if none
   */
  std::string getCurrentMusic() const { return m_currentMusic; }

private:
  std::shared_ptr<IRenderer> m_renderer;

  // Audio resources
  std::unordered_map<std::string, void *> m_sounds;
  std::unordered_map<std::string, void *> m_music;

  // Current state
  std::string m_currentMusic;
  int m_masterVolume = 100;
  int m_musicVolume = 100;
  int m_sfxVolume = 100;

  /**
   * @brief Load a sound file
   * @param name Internal name for the sound
   * @param filepath Path to the sound file
   * @return true if loading succeeded
   */
  bool loadSound(const std::string &name, const std::string &filepath);

  /**
   * @brief Load a music file
   * @param name Internal name for the music
   * @param filepath Path to the music file
   * @return true if loading succeeded
   */
  bool loadMusicTrack(const std::string &name, const std::string &filepath);

  /**
   * @brief Apply current volume settings to renderer
   */
  void updateVolumes();
};
