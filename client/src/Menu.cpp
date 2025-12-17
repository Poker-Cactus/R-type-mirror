/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Menu.cpp
*/

#include "Menu.hpp"
#include "../include/AssetPath.hpp"
#include "Menu/LoadingMenu/LoadingMenu.hpp"
#include "Menu/LobbyMenu/LobbyMenu.hpp"
#include "Menu/MainMenu/MainMenu.hpp"
#include "Menu/ProfileMenu/ProfileMenu.hpp"
#include "Menu/SettingsMenu/SettingsMenu.hpp"
#include <cmath>
#include <iostream>

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
    menu_font = renderer->loadFont(resolveAssetPath("client/assets/font.opf/r-type.otf").c_str(), menuFontSize);

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
    std::cerr << "Exception during menu initialization: " << e.what() << '\n';
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
    lobbyMenu->render({.width = winWidth, .height = winHeight}, renderer);
    break;
  case MenuState::SETTINGS:
    settingsMenu->render(winWidth, winHeight, renderer);
    break;
  case MenuState::EXIT:
    // Exit state - nothing to render
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
    lobbyMenu->process(renderer, &currentState);
    break;
  default:
    break;
  }
  processBack();
}

void Menu::cleanup() {}

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

  int pos_x = (winWidth - textWidth) / 2;
  int pos_y = ((winHeight - textHeight) / 2) + yOffset;

  renderer->drawText(menu_font, text, pos_x, pos_y, color);
}

MenuState Menu::getState() const
{
  return currentState;
}

bool Menu::shouldStartGame() const
{
  // Only transition to lobby room when user explicitly chooses to create/join
  if (lobbyMenu != nullptr && lobbyMenu->shouldEnterLobbyRoom()) {
    std::cout << "[Menu] shouldStartGame() returning true - User selected lobby action" << '\n';
    return true;
  }
  return false;
}

bool Menu::isCreatingLobby() const
{
  return lobbyMenu != nullptr && lobbyMenu->isCreatingLobby();
}

std::string Menu::getLobbyCodeToJoin() const
{
  if (lobbyMenu != nullptr) {
    return lobbyMenu->getLobbyCodeToJoin();
  }
  return "";
}

void Menu::resetLobbySelection()
{
  if (lobbyMenu != nullptr) {
    lobbyMenu->resetLobbyRoomFlag();
  }
}
