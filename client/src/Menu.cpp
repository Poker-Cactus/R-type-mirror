/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Menu.cpp
*/

#include "Menu.hpp"
#include <cmath>

Menu::Menu(IRenderer *renderer) : renderer(renderer), backgroundTexture(nullptr), blinkTimer(0.0f) {}

Menu::~Menu()
{
    cleanup();
}

void Menu::init()
{
    try {
        const int menuFontSize = 24;
        const int titleFontSize = 48;
        logo = renderer->loadTexture("client/assets/logoRTYPE.png");
        backgroundTexture = renderer->loadTexture("client/assets/background/starfield.png");
        menu_font = renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);
        title_font = renderer->loadFont("client/assets/font.opf/r-type.otf", titleFontSize);
        planet = renderer->loadTexture("client/assets/moon-pack/moon1.png");

        // Initialiser le LoadingScreen
        loadingScreen = new LoadingScreen(renderer, menu_font);
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
    case MenuState::PROFILE:
        renderProfile(winWidth, winHeight);
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
        this->processLoading();
        break;
    case MenuState::MAIN_MENU:
        this->processMainMenu();
        break;
    case MenuState::PROFILE:
        this->processProfile();
        break;
    case MenuState::SETTINGS:
        this->processSettings();
        break;
    case MenuState::LOBBY:
        this->processLobby();
        break;
    default:
        break;
    }
    processBack();
}

void Menu::cleanup()
{
    if (backgroundTexture != nullptr) {
        renderer->freeTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
    if (loadingScreen != nullptr) {
        delete loadingScreen;
        loadingScreen = nullptr;
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
