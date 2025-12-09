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

void Menu::drawThickBorderedRect(int x, int y, int width, int height, const Color &color, int thickness)
{
    for (int i = 0; i < thickness; i++) {
        renderer->drawRectOutline(x - i, y - i, width + (2 * i), height + (2 * i), color);
    }
}

void Menu::renderSettings(int winWidth, int winHeight)
{
    std::string SettingsTitle = "Settings";
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(title_font, SettingsTitle, textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = textHeight;

    renderer->drawText(title_font, SettingsTitle, x, y, {255, 255, 255});
}

void Menu::renderLoading(int winWidth, int winHeight)
{
    if (loadingScreen != nullptr && loadingScreen->isActive()) {
        if (loadingScreen->update(winWidth, winHeight)) {
            currentState = MenuState::MAIN_MENU;
        }
        return;
    }

    if (isZooming) {
        zoomTimer += renderer->getDeltaTime();

        float progress = zoomTimer / zoomDuration;

        if (progress >= 1.0f) {
            isZooming = false;
            zoomTimer = 0.0f;
            zoomScale = 0.3f;
            if (loadingScreen != nullptr) {
                loadingScreen->start();
            }
            return;
        }

        zoomScale = 0.3f + (progress * progress * 9.7f);
    }

    backgroundOffsetX += renderer->getDeltaTime() * 20.0f;
    if (backgroundOffsetX >= winWidth) {
        backgroundOffsetX = 0.0f;
    }

    if (backgroundTexture != nullptr) {
        renderer->drawTextureEx(backgroundTexture, static_cast<int>(backgroundOffsetX), 0, winWidth, winHeight, 0.0,
                                false, false);
        renderer->drawTextureEx(backgroundTexture, static_cast<int>(backgroundOffsetX - winWidth), 0, winWidth,
                                winHeight, 0.0, false, false);
    }
    if (planet != nullptr) {
        int planetWidth, planetHeight;
        renderer->getTextureSize(planet, planetWidth, planetHeight);

        float scale = static_cast<float>(std::min(winWidth, winHeight)) * zoomScale /
            static_cast<float>(std::max(planetWidth, planetHeight));

        int scaledWidth = static_cast<int>(planetWidth * scale);
        int scaledHeight = static_cast<int>(planetHeight * scale);

        int planetX = (winWidth - scaledWidth) / 2;
        int planetY = (winHeight - scaledHeight) / 2;

        renderer->drawTextureEx(planet, planetX, planetY, scaledWidth, scaledHeight, 0.0, false, false);
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

        renderer->drawText(menu_font, text, x, y, {255, 255, 255, static_cast<unsigned char>(alpha)});
    }
}