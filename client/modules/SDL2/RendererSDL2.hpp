#pragma once
#include "../../interface/IRenderer.hpp"
#include <SDL2/SDL.h>
#include <map>
#include <vector>

class RendererSDL2 : public IRenderer
{
  public:
    RendererSDL2(int width = 800, int height = 600);
    ~RendererSDL2() override;

    // === IRender ===
    void clear(const Color &color = {0, 0, 0, 255}) override;
    void present() override;

    // === IWindow ===
    int getWindowWidth() const override { return windowWidth; }
    int getWindowHeight() const override { return windowHeight; }
    void setWindowSize(int width, int height) override;
    void setWindowTitle(const std::string &title) override;
    void setFullscreen(bool fullscreen) override;
    bool isFullscreen() const override;
    bool pollEvents() override;

    // === IInput ===
    bool isKeyPressed(int keycode) override;
    bool isKeyReleased(int keycode) override;
    bool isKeyJustPressed(int keycode) override;
    void getMousePosition(int &x, int &y) override;
    bool isMouseButtonPressed(int button) override;
    int getNumGamepads() override;
    bool isGamepadButtonPressed(int gamepadIndex, int button) override;
    float getGamepadAxis(int gamepadIndex, int axis) override;

    // === ITexture ===
    void *loadTexture(const std::string &filepath) override;
    void freeTexture(void *texture) override;
    void getTextureSize(void *texture, int &width, int &height) override;
    void drawTexture(void *texture, int x, int y) override;
    void drawTextureRegion(void *texture, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY, int dstW,
                           int dstH) override;
    void drawTextureEx(void *texture, int x, int y, int w, int h, double angle, bool flipX, bool flipY) override;

    // === IText ===
    void *loadFont(const std::string &filepath, int fontSize) override;
    void freeFont(void *font) override;
    void drawText(void *font, const std::string &text, int x, int y, const Color &color) override;
    void getTextSize(void *font, const std::string &text, int &width, int &height) override;

    // === IAudio ===
    void *loadSound(const std::string &filepath) override;
    void *loadMusic(const std::string &filepath) override;
    void playSound(void *sound, int loops) override;
    void playMusic(void *music, int loops) override;
    void pauseMusic() override;
    void resumeMusic() override;
    void stopMusic() override;
    void setSoundVolume(int volume) override;
    void setMusicVolume(int volume) override;
    void freeSound(void *sound) override;
    void freeMusic(void *music) override;

    // === IShape ===
    void drawRect(int x, int y, int w, int h, const Color &color) override;
    void drawRectOutline(int x, int y, int w, int h, const Color &color) override;
    void drawLine(int x1, int y1, int x2, int y2, const Color &color) override;
    void drawCircle(int centerX, int centerY, int radius, const Color &color) override;
    void drawCircleFilled(int centerX, int centerY, int radius, const Color &color) override;
    void drawPoint(int x, int y, const Color &color) override;

    // === ICamera ===
    void setViewport(int x, int y, int w, int h) override;
    void resetViewport() override;
    void setCameraOffset(int offsetX, int offsetY) override;
    void getCameraOffset(int &offsetX, int &offsetY) const override;

    // === ITime ===
    float getDeltaTime() const override;
    int getFPS() const override;
    void setTargetFPS(int fps) override;
    void setVSync(bool enabled) override;

    // === ICollision ===
    bool checkCollisionRects(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) override;
    bool checkCollisionCircles(int x1, int y1, int r1, int x2, int y2, int r2) override;
    bool checkPointInRect(int px, int py, int rx, int ry, int rw, int rh) override;

  private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    int windowWidth;
    int windowHeight;
    bool fullscreen = false;

    // Timing
    Uint64 lastFrameTime = 0;
    float deltaTime = 0.0f;
    int targetFPS = 60;
    int currentFPS = 0;

    // Camera
    int cameraOffsetX = 0;
    int cameraOffsetY = 0;

    // Input state
    std::map<int, bool> keyStates;
    std::map<int, bool> previousKeyStates;
    std::map<int, bool> mouseButtonStates;
    int mouseX = 0;
    int mouseY = 0;

    // Gamepads
    std::vector<SDL_GameController *> gamepads;
};
