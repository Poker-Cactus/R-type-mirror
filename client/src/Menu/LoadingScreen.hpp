#pragma once
#include "../../interface/IRenderer.hpp"

class LoadingScreen
{
    public:
    LoadingScreen(IRenderer *renderer, void *font);
    ~LoadingScreen() = default;

    void start();
    void stop();
    bool update(int winWidth, int winHeight);
    bool isActive() const;
    void setDuration(float duration);

    private:
    IRenderer *renderer;
    void *font;
    bool active = false;
    float timer = 0.0f;
    float duration = 2.0f;
    float rotation = 0.0f;
};