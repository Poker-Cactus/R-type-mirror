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

Menu::Menu(IRenderer *renderer) : m_renderer(renderer) {}

Menu::~Menu()
{
  cleanup();
}

void Menu::init()
{
  try {
    const int menuFontSize = 24;
    // const int titleFontSize = 48;
    menu_font = m_renderer->loadFont(resolveAssetPath("client/assets/font.opf/r-type.otf").c_str(), menuFontSize);

    m_loadingMenu = std::make_shared<LoadingMenu>(m_renderer);
    m_loadingMenu->init();

    // Initialiser MainMenu
    m_mainMenu = std::make_shared<MainMenu>(m_renderer);
    m_mainMenu->init();

    m_lobbyMenu = std::make_shared<LobbyMenu>(m_renderer);
    m_lobbyMenu->init();

    m_profileMenu = std::make_shared<ProfileMenu>(m_renderer);
    m_profileMenu->init();

    m_settingsMenu = std::make_shared<SettingsMenu>(m_renderer);
    m_settingsMenu->init();

    moonSky = m_renderer->loadTexture("client/assets/moon-para/moon_sky.png");
    moonBack = m_renderer->loadTexture("client/assets/moon-para/moon_back.png");
    moonMid = m_renderer->loadTexture("client/assets/moon-para/moon_mid.png");
    moonFront = m_renderer->loadTexture("client/assets/moon-para/moon_front.png");
    moonFloor = m_renderer->loadTexture("client/assets/moon-para/moon_floor.png");

    // Initialiser le LoadingScreen
    loadingScreen = std::make_shared<LoadingScreen>(m_renderer, menu_font);
  } catch (const std::exception &e) {
    std::cerr << "Exception during menu initialization: " << e.what() << '\n';
  }
}

void Menu::render()
{
  int winWidth = m_renderer->getWindowWidth();
  int winHeight = m_renderer->getWindowHeight();

  switch (currentState) {
  case MenuState::LOADING:
    m_loadingMenu->render(winWidth, winHeight, loadingScreen.get(), &currentState);
    break;
  case MenuState::MAIN_MENU:
    renderMoonParalax(winWidth, winHeight);
    m_mainMenu->render(winWidth, winHeight);
    break;
  case MenuState::PROFILE:
    renderMoonParalax(winWidth, winHeight);
    m_profileMenu->render(winWidth, winHeight);
    break;
  case MenuState::LOBBY:
    m_lobbyMenu->render({.width = winWidth, .height = winHeight});
    break;
  case MenuState::SETTINGS:
    renderMoonParalax(winWidth, winHeight);
    m_settingsMenu->render(winWidth, winHeight);
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
    m_loadingMenu->process();
    break;
  case MenuState::MAIN_MENU:
    m_mainMenu->process(&currentState);
    break;
  case MenuState::PROFILE:
    m_profileMenu->process();
    break;
  case MenuState::SETTINGS:
    m_settingsMenu->process();
    break;
  case MenuState::LOBBY:
    m_lobbyMenu->process(&currentState);
    break;
  default:
    break;
  }
  processBack();
}

void Menu::cleanup()
{
  if (menu_font != nullptr && m_renderer != nullptr)
    m_renderer->freeFont(menu_font);
  if (moonFloor != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(moonFloor);
  if (moonSky != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(moonSky);
  if (moonMid != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(moonMid);
  if (moonFront != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(moonFront);
  if (moonBack != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(moonBack);
  menu_font = nullptr;
  moonFloor = nullptr;
  moonSky = nullptr;
  moonMid = nullptr;
  moonFront = nullptr;
  moonBack = nullptr;
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

  int winWidth = m_renderer->getWindowWidth();
  int winHeight = m_renderer->getWindowHeight();

  int textWidth = 0;
  int textHeight = 0;
  m_renderer->getTextSize(menu_font, text, textWidth, textHeight);

  int pos_x = (winWidth - textWidth) / 2;
  int pos_y = ((winHeight - textHeight) / 2) + yOffset;

  m_renderer->drawText(menu_font, text, pos_x, pos_y, color);
}

MenuState Menu::getState() const
{
  return currentState;
}

bool Menu::shouldStartGame() const
{
  // Only transition to lobby room when user explicitly chooses to create/join
  if (m_lobbyMenu != nullptr && m_lobbyMenu->shouldEnterLobbyRoom()) {
    std::cout << "[Menu] shouldStartGame() returning true - User selected lobby action" << '\n';
    return true;
  }
  return false;
}

bool Menu::isCreatingLobby() const
{
  return m_lobbyMenu != nullptr && m_lobbyMenu->isCreatingLobby();
}

std::string Menu::getLobbyCodeToJoin() const
{
  if (m_lobbyMenu != nullptr) {
    return m_lobbyMenu->getLobbyCodeToJoin();
  }
  return "";
}

void Menu::resetLobbySelection()
{
  if (m_lobbyMenu != nullptr) {
    m_lobbyMenu->resetLobbyRoomFlag();
  }
}

void Menu::renderMoonParalax(int winWidth, int winHeight)
{
  float deltaTime = m_renderer->getDeltaTime();

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
    m_renderer->drawTextureEx(moonSky, static_cast<int>(parallaxOffsetSky), 0, winWidth, winHeight, 0.0, false, false);
    m_renderer->drawTextureEx(moonSky, static_cast<int>(parallaxOffsetSky - winWidth), 0, winWidth, winHeight, 0.0, false,
                            false);
  }

  if (moonBack != nullptr) {
    m_renderer->drawTextureEx(moonBack, static_cast<int>(parallaxOffsetBack), 0, winWidth, winHeight, 0.0, false, false);
    m_renderer->drawTextureEx(moonBack, static_cast<int>(parallaxOffsetBack - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (moonMid != nullptr) {
    m_renderer->drawTextureEx(moonMid, static_cast<int>(parallaxOffsetMid), 0, winWidth, winHeight, 0.0, false, false);
    m_renderer->drawTextureEx(moonMid, static_cast<int>(parallaxOffsetMid - winWidth), 0, winWidth, winHeight, 0.0, false,
                            false);
  }

  if (moonFront != nullptr) {
    m_renderer->drawTextureEx(moonFront, static_cast<int>(parallaxOffsetFront), 0, winWidth, winHeight, 0.0, false,
                            false);
    m_renderer->drawTextureEx(moonFront, static_cast<int>(parallaxOffsetFront - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (moonFloor != nullptr) {
    m_renderer->drawTextureEx(moonFloor, static_cast<int>(parallaxOffsetFloor), 0, winWidth, winHeight, 0.0, false,
                            false);
    m_renderer->drawTextureEx(moonFloor, static_cast<int>(parallaxOffsetFloor - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }
}
