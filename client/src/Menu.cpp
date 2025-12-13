/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Menu.cpp
*/

#include "Menu.hpp"
#include "Menu/LoadingMenu/LoadingMenu.hpp"
#include "Menu/LobbyMenu/LobbyMenu.hpp"
#include "Menu/MainMenu/MainMenu.hpp"
#include "Menu/ProfileMenu/ProfileMenu.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
#include <cmath>

Menu::Menu(IRenderer *renderer) : renderer(renderer) {}

Menu::~Menu()
{
    cleanup();
}

void Menu::init()
{
    try {
        const int menuFontSize = 24;
        const int titleFontSize = 48;
        menu_font = renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);

        loadingMenu = new LoadingMenu();
        loadingMenu->init(renderer);

        // Initialiser MainMenu
        mainMenu = new MainMenu();
        mainMenu->init(renderer);

        lobbyMenu = new LobbyMenu();
        lobbyMenu->init(renderer);

        profileMenu = new ProfileMenu();
        profileMenu->init(renderer);

        settingsMenu = new SettingsMenu();
        settingsMenu->init(renderer);

        // Initialiser le LoadingScreen
        loadingScreen = new LoadingScreen(renderer, menu_font);
    } catch (const std::exception &e) {
    }
}

void Menu::render()
{
    int winWidth = renderer->getWindowWidth();
    int winHeight = renderer->getWindowHeight();

    switch (currentState) {
    case MenuState::LOADING:
        loadingMenu->render(winWidth, winHeight, renderer, loadingScreen, &currentState);
        break;
    case MenuState::MAIN_MENU:
        mainMenu->render(winWidth, winHeight, renderer);
        break;
    case MenuState::PROFILE:
        profileMenu->render(winWidth, winHeight, renderer);
        break;
    case MenuState::LOBBY:
        lobbyMenu->render(winWidth, winHeight, renderer);
        break;
    case MenuState::SETTINGS:
        settingsMenu->render(winWidth, winHeight, renderer);
        break;
    }
}

void Menu::processInput()
{
    switch (currentState) {
    case MenuState::LOADING:
        loadingMenu->process(renderer);
        break;
    case MenuState::MAIN_MENU:
        mainMenu->process(&currentState, renderer);
        break;
    case MenuState::PROFILE:
        profileMenu->process(renderer);
        break;
    case MenuState::SETTINGS:
        settingsMenu->process(renderer);
        break;
    case MenuState::LOBBY:
        lobbyMenu->process(renderer);
        break;
    default:
        break;
    }
    processBack();
}

void Menu::cleanup()
{

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

bool Menu::shouldStartGame() const
{
    return currentState == MenuState::LOBBY;
}
