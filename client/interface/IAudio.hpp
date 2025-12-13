#pragma once
#include <string>

class IAudio
{
  public:
    virtual ~IAudio() = default;

    virtual void *loadSound(const std::string &filepath) = 0;
    virtual void *loadMusic(const std::string &filepath) = 0;
    virtual void playSound(void *sound, int loops = 0) = 0;
    virtual void playMusic(void *music, int loops = -1) = 0;
    virtual void pauseMusic() = 0;
    virtual void resumeMusic() = 0;
    virtual void stopMusic() = 0;
    virtual void setSoundVolume(int volume) = 0;
    virtual void setMusicVolume(int volume) = 0;
    virtual void freeSound(void *sound) = 0;
    virtual void freeMusic(void *music) = 0;
};
