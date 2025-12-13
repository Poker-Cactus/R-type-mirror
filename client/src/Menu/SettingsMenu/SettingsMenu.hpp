#pragma once
#include "../../../interface/IRenderer.hpp"

class SettingsMenu {
    public:
        SettingsMenu() = default;
        ~SettingsMenu() {};
        void init(IRenderer *renderer);
        void render(int winWidth, int winHeight, IRenderer *renderer);
        void process(IRenderer *renderer);
    private:
};