/**
 * @file AudioManager.cpp
 * @brief Implementation of centralized audio management system
 */

#include "../include/AudioManager.hpp"
#include <algorithm>
#include <iostream>

AudioManager::AudioManager(std::shared_ptr<IRenderer> renderer) : m_renderer(std::move(renderer)) {}

AudioManager::~AudioManager()
{
  cleanup();
}

bool AudioManager::init()
{
  if (!m_renderer) {
    std::cerr << "[AudioManager] ERROR: Renderer is null" << '\n';
    return false;
  }

  std::cout << "[AudioManager] Initializing audio system..." << '\n';

  // Load all sound effects
  loadSound("base_shot", "client/assets/Sounds/rtype_base_shot.mp3");
  loadSound("charged_shot", "client/assets/Sounds/charged_shot.mp3");
  loadSound("enemy_explosion", "client/assets/Sounds/enemy_explosion.wav");
  loadSound("button_click", "client/assets/Sounds/buttonClicked.wav");
  loadSound("button_hover", "client/assets/Sounds/Hovering1.wav");

  // Load all music tracks
  loadMusicTrack("menu_music", "client/assets/audios/loadingMusic.mp3");
  loadMusicTrack("level1_music", "client/assets/Sounds/level1.mp3");
  loadMusicTrack("intro_music", "client/assets/audios/loadingMusic.mp3");

  std::cout << "[AudioManager] Loaded " << m_sounds.size() << " sounds and " << m_music.size() << " music tracks"
            << '\n';

  return true;
}

void AudioManager::cleanup()
{
  if (!m_renderer) {
    return;
  }

  // Stop any playing music
  stopMusic();

  // Free all sounds
  for (auto &[name, sound] : m_sounds) {
    if (sound) {
      m_renderer->freeSound(sound);
    }
  }
  m_sounds.clear();

  // Free all music
  for (auto &[name, music] : m_music) {
    if (music) {
      m_renderer->freeMusic(music);
    }
  }
  m_music.clear();

  std::cout << "[AudioManager] Cleaned up audio resources" << '\n';
}

bool AudioManager::loadSound(const std::string &name, const std::string &filepath)
{
  if (!m_renderer) {
    return false;
  }

  try {
    void *sound = m_renderer->loadSound(filepath);
    if (sound != nullptr) {
      m_sounds[name] = sound;
      std::cout << "[AudioManager] ✓ Loaded sound: " << name << " (" << filepath << ")" << '\n';
      return true;
    } else {
      std::cerr << "[AudioManager] ✗ Failed to load sound: " << name << " (" << filepath << ")" << '\n';
      return false;
    }
  } catch (const std::exception &e) {
    std::cerr << "[AudioManager] ✗ Exception loading sound " << name << ": " << e.what() << '\n';
    return false;
  }
}

bool AudioManager::loadMusicTrack(const std::string &name, const std::string &filepath)
{
  if (!m_renderer) {
    return false;
  }

  try {
    void *music = m_renderer->loadMusic(filepath);
    if (music != nullptr) {
      m_music[name] = music;
      std::cout << "[AudioManager] ✓ Loaded music: " << name << " (" << filepath << ")" << '\n';
      return true;
    } else {
      std::cerr << "[AudioManager] ✗ Failed to load music: " << name << " (" << filepath << ")" << '\n';
      return false;
    }
  } catch (const std::exception &e) {
    std::cerr << "[AudioManager] ✗ Exception loading music " << name << ": " << e.what() << '\n';
    return false;
  }
}

void AudioManager::playSound(const std::string &soundName, int loops)
{
  if (!m_renderer) {
    std::cerr << "[AudioManager] ERROR: playSound called but renderer is null" << '\n';
    return;
  }

  auto it = m_sounds.find(soundName);
  if (it != m_sounds.end() && it->second != nullptr) {
    std::cout << "[AudioManager] Playing sound: " << soundName << " (loops: " << loops << ")" << '\n';
    m_renderer->playSound(it->second, loops);
  } else {
    std::cerr << "[AudioManager] WARNING: Sound not found: " << soundName << '\n';
  }
}

void AudioManager::playMusic(const std::string &musicName, bool fadeOut)
{
  if (!m_renderer) {
    std::cerr << "[AudioManager] ERROR: playMusic called but renderer is null" << '\n';
    return;
  }

  // Stop current music if playing
  if (fadeOut && !m_currentMusic.empty()) {
    stopMusic();
  }

  auto it = m_music.find(musicName);
  if (it != m_music.end() && it->second != nullptr) {
    std::cout << "[AudioManager] Playing music: " << musicName << " (master: " << m_masterVolume
              << ", music: " << m_musicVolume << ")" << '\n';
    m_renderer->playMusic(it->second, -1); // Loop infinitely
    m_currentMusic = musicName;
    updateVolumes();
  } else {
    std::cerr << "[AudioManager] WARNING: Music not found: " << musicName << '\n';
  }
}

void AudioManager::stopMusic()
{
  if (!m_renderer) {
    return;
  }

  m_renderer->stopMusic();
  m_currentMusic.clear();
}

void AudioManager::pauseMusic()
{
  if (!m_renderer) {
    return;
  }

  m_renderer->pauseMusic();
}

void AudioManager::resumeMusic()
{
  if (!m_renderer) {
    return;
  }

  m_renderer->resumeMusic();
}

void AudioManager::setMasterVolume(int volume)
{
  m_masterVolume = std::clamp(volume, 0, 100);
  updateVolumes();
}

void AudioManager::setMusicVolume(int volume)
{
  m_musicVolume = std::clamp(volume, 0, 100);
  updateVolumes();
}

void AudioManager::setSfxVolume(int volume)
{
  m_sfxVolume = std::clamp(volume, 0, 100);
  updateVolumes();
}

void AudioManager::updateVolumes()
{
  if (!m_renderer) {
    return;
  }

  // Calculate effective volumes (master volume affects all)
  int effectiveMusicVolume = (m_musicVolume * m_masterVolume) / 100;
  int effectiveSfxVolume = (m_sfxVolume * m_masterVolume) / 100;

  std::cout << "[AudioManager] Setting volumes - Music: " << effectiveMusicVolume << ", SFX: " << effectiveSfxVolume
            << '\n';
  m_renderer->setMusicVolume(effectiveMusicVolume);
  m_renderer->setSoundVolume(effectiveSfxVolume);
}

bool AudioManager::isSoundLoaded(const std::string &soundName) const
{
  return m_sounds.find(soundName) != m_sounds.end();
}

bool AudioManager::isMusicLoaded(const std::string &musicName) const
{
  return m_music.find(musicName) != m_music.end();
}
