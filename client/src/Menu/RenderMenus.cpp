/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** RenderMenus.cpp
*/

#include "../Menu.hpp"
#include <cmath>

void Menu::renderMainMenu(int winWidth, int winHeight)
{
    if (menu_font == nullptr) {
        return;
    }

    for (size_t i = 0; i < mainMenuItems.size(); i++) {
        int textWidth = 0;
        int textHeight = 0;
        renderer->getTextSize(menu_font, mainMenuItems[i], textWidth, textHeight);

        int x = (winWidth - textWidth) / 2;
        int y = (winHeight / 2) + (static_cast<int>(i) * 60) - 90;

        Color color =
            (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
        renderer->drawText(menu_font, mainMenuItems[i], x, y, color);
    }
}

void Menu::renderProfile(int winWidth, int winHeight)
{
    // TODO: Afficher le menu des options
    drawCenteredText("OPTIONS - Coming soon", 0, {255, 255, 255, 255});
}

void Menu::renderLobby(int winWidth, int winHeight)
{
    // TODO: Afficher le lobby
    drawCenteredText("LOBBY - Coming soon", 0, {255, 255, 255, 255});
}

void Menu::renderSettings(int winWidth, int winHeight)
{
    // TODO: Afficher les settings
    drawCenteredText("SETTINGS - Coming soon", 0, {255, 255, 255, 255});
}

void Menu::renderLoading(int winWidth, int winHeight)
{
    // Afficher le logo
    if (logo != nullptr) {
        int logoWidth = winWidth * 40 / 100;
        int logoHeight = logoWidth / 3;

        int logoX = (winWidth - logoWidth) / 2;
        int logoY = winHeight / 10;
        renderer->drawTextureEx(logo, logoX, logoY, logoWidth, logoHeight, 0.0, false, false);
    }

    if (menu_font != nullptr) {
        blinkTimer += renderer->getDeltaTime();
        float opacity = (std::sin(blinkTimer * 3.5f) + 1.0f) / 2.0f;
        int alpha = static_cast<int>(50 + opacity * 205);

        std::string text = "Press enter to start ...";
        int textWidth = 0;
        int textHeight = 0;
        renderer->getTextSize(menu_font, text, textWidth, textHeight);

        int x = (winWidth - textWidth) / 2;
        int y = (winHeight - textHeight) / 1.1;

        renderer->drawText(menu_font, text, x, y, {4, 196, 199, static_cast<unsigned char>(alpha)});
    }
}