/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** MainMenu.cpp
*/

#include "MainMenu.hpp"
#include "../../../interface/KeyCodes.hpp"

void MainMenu::init(IRenderer *renderer)
{
    try {
        const int menuFontSize = 24;
        planet = renderer->loadTexture("client/assets/moon-pack/moon1.png");
        moonSky = renderer->loadTexture("client/assets/moon-para/moon_sky.png");
        moonBack = renderer->loadTexture("client/assets/moon-para/moon_back.png");
        moonMid = renderer->loadTexture("client/assets/moon-para/moon_mid.png");
        moonFront = renderer->loadTexture("client/assets/moon-para/moon_front.png");
        moonFloor = renderer->loadTexture("client/assets/moon-para/moon_floor.png");
        font = renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);

    } catch (const std::exception &e) {
        planet = nullptr;
        moonSky = nullptr;
        moonBack = nullptr;
        moonMid = nullptr;
        moonFront = nullptr;
        moonFloor = nullptr;
    }
}

void MainMenu::render(int winWidth, int winHeight, IRenderer *renderer)
{
    if (font == nullptr) {
        return;
    }

    float deltaTime = renderer->getDeltaTime();

    parallaxOffsetSky += deltaTime * 5.0f;
    parallaxOffsetBack += deltaTime * 15.0f;
    parallaxOffsetMid += deltaTime * 30.0f;
    parallaxOffsetFront += deltaTime * 50.0f;
    parallaxOffsetFloor += deltaTime * 70.0f;

    if (parallaxOffsetSky >= winWidth)
        parallaxOffsetSky = 0.0f;
    if (parallaxOffsetBack >= winWidth)
        parallaxOffsetBack = 0.0f;
    if (parallaxOffsetMid >= winWidth)
        parallaxOffsetMid = 0.0f;
    if (parallaxOffsetFront >= winWidth)
        parallaxOffsetFront = 0.0f;
    if (parallaxOffsetFloor >= winWidth)
        parallaxOffsetFloor = 0.0f;

    if (moonSky != nullptr) {
        renderer->drawTextureEx(moonSky, static_cast<int>(parallaxOffsetSky), 0, winWidth, winHeight, 0.0, false,
                                false);
        renderer->drawTextureEx(moonSky, static_cast<int>(parallaxOffsetSky - winWidth), 0, winWidth, winHeight, 0.0,
                                false, false);
    }

    if (moonBack != nullptr) {
        renderer->drawTextureEx(moonBack, static_cast<int>(parallaxOffsetBack), 0, winWidth, winHeight, 0.0, false,
                                false);
        renderer->drawTextureEx(moonBack, static_cast<int>(parallaxOffsetBack - winWidth), 0, winWidth, winHeight, 0.0,
                                false, false);
    }

    if (moonMid != nullptr) {
        renderer->drawTextureEx(moonMid, static_cast<int>(parallaxOffsetMid), 0, winWidth, winHeight, 0.0, false,
                                false);
        renderer->drawTextureEx(moonMid, static_cast<int>(parallaxOffsetMid - winWidth), 0, winWidth, winHeight, 0.0,
                                false, false);
    }

    if (moonFront != nullptr) {
        renderer->drawTextureEx(moonFront, static_cast<int>(parallaxOffsetFront), 0, winWidth, winHeight, 0.0, false,
                                false);
        renderer->drawTextureEx(moonFront, static_cast<int>(parallaxOffsetFront - winWidth), 0, winWidth, winHeight,
                                0.0, false, false);
    }

    if (moonFloor != nullptr) {
        renderer->drawTextureEx(moonFloor, static_cast<int>(parallaxOffsetFloor), 0, winWidth, winHeight, 0.0, false,
                                false);
        renderer->drawTextureEx(moonFloor, static_cast<int>(parallaxOffsetFloor - winWidth), 0, winWidth, winHeight,
                                0.0, false, false);
    }

    for (size_t i = 0; i < mainMenuItems.size(); i++) {
        int textWidth = 0;
        int textHeight = 0;
        renderer->getTextSize(font, mainMenuItems[i], textWidth, textHeight);

        int x = (winWidth - textWidth) / 2;
        int y = (winHeight / 2) + (static_cast<int>(i) * 60) - 90;

        Color color =
            (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
        renderer->drawText(font, mainMenuItems[i], x, y, color);
    }
}

void MainMenu::process(MenuState *currentState, IRenderer *renderer)
{
    // Flèche bas - descendre dans le menu
    if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
        currentMenuIndex = (currentMenuIndex + 1) % mainMenuItems.size();
    }
    // Flèche haut - monter dans le menu
    if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
        currentMenuIndex = (currentMenuIndex - 1 + mainMenuItems.size()) % mainMenuItems.size();
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
        std::string selectedButton = mainMenuItems[currentMenuIndex];
        if (selectedButton == "Play") {
            *currentState = MenuState::LOBBY;
        } else if (selectedButton == "Settings") {
            *currentState = MenuState::SETTINGS;
        } else if (selectedButton == "Profile") {
            *currentState = MenuState::PROFILE;
        } else {
            *currentState = MenuState::EXIT;
        }
    }
}