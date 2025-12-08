/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Menu.cpp
*/

#include "Menu.hpp"
#include "../interface/KeyCodes.hpp"
#include <cmath>
#include <iostream>

Menu::Menu(IRenderer *renderer) : renderer(renderer), backgroundTexture(nullptr), blinkTimer(0.0f) {}

Menu::~Menu()
{
    cleanup();
}

void Menu::init()
{
    try {
        logo = renderer->loadTexture("client/assets/logoRTYPE.png");
        menu_font = renderer->loadFont("client/assets/font.opf/r-type.otf", 24);
    } catch (const std::exception &e) {
        backgroundTexture = nullptr;
    }
}

void Menu::render()
{
    int winWidth = renderer->getWindowWidth();
    int winHeight = renderer->getWindowHeight();

    switch (currentState) {
    case MenuState::LOADING:
        renderLoading(winWidth, winHeight);
        break;
    case MenuState::MAIN_MENU:
        renderMainMenu(winWidth, winHeight);
        break;
    case MenuState::OPTIONS:
        renderOptions(winWidth, winHeight);
        break;
    case MenuState::LOBBY:
        renderLobby(winWidth, winHeight);
        break;
    case MenuState::SETTINGS:
        renderSettings(winWidth, winHeight);
        break;
    }
}

void Menu::processInput()
{
    switch (currentState) {
    case MenuState::LOADING:
        if (renderer->isKeyPressed(KeyCode::KEY_RETURN))
            this->currentState = MenuState::MAIN_MENU;
        break;
    case MenuState::MAIN_MENU:
        // Flèche bas - descendre dans le menu
        if (renderer->isKeyPressed(KeyCode::KEY_DOWN)) {
            currentMenuIndex += 1;
            std::cout << "Current index: " << currentMenuIndex << std::endl;
        }
        // Flèche haut - monter dans le menu
        if (renderer->isKeyPressed(KeyCode::KEY_UP)) {
        }
        break;
    default:
        break;
    }
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

    // Afficher le texte clignotant
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

        // Couleur différente si c'est l'élément sélectionné
        Color color =
            (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
        renderer->drawText(menu_font, mainMenuItems[i], x, y, color);
    }
}

void Menu::renderOptions(int winWidth, int winHeight)
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

void Menu::cleanup()
{
    if (backgroundTexture != nullptr) {
        renderer->freeTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
}

void Menu::setState(MenuState newState)
{
    currentState = newState;
}

void Menu::drawCenteredText(const std::string &text, int yOffset, const Color &color)
{
    if (menu_font == nullptr) {
        return;
    }

    int winWidth = renderer->getWindowWidth();
    int winHeight = renderer->getWindowHeight();

    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(menu_font, text, textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight - textHeight) / 2 + yOffset;

    renderer->drawText(menu_font, text, x, y, color);
}

MenuState Menu::getState() const
{
    return currentState;
}
