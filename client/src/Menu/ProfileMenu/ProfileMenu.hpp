#pragma once
#include <array>
#include <string>
#include "../../../interface/IRenderer.hpp"
#include "../MenuState.hpp"

class ProfileMenu {
    public:
        ProfileMenu() = default;
        ~ProfileMenu() {};
        void init(IRenderer *renderer);
        void render(int winWidth, int winHeight, IRenderer *renderer);
        void process(IRenderer *renderer);
    private:
};