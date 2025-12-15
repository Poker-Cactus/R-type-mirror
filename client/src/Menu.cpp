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

    moonSky = renderer->loadTexture("client/assets/moon-para/moon_sky.png");
    moonBack = renderer->loadTexture("client/assets/moon-para/moon_back.png");
    moonMid = renderer->loadTexture("client/assets/moon-para/moon_mid.png");
    moonFront = renderer->loadTexture("client/assets/moon-para/moon_front.png");
    moonFloor = renderer->loadTexture("client/assets/moon-para/moon_floor.png");

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
    renderMoonParalax(winWidth, winHeight, renderer);
    mainMenu->render(winWidth, winHeight, renderer);
    break;
  case MenuState::PROFILE:
    renderMoonParalax(winWidth, winHeight, renderer);
    profileMenu->render(winWidth, winHeight, renderer);
    break;
  case MenuState::LOBBY:
    lobbyMenu->render(winWidth, winHeight, renderer);
    break;
  case MenuState::SETTINGS:
    renderMoonParalax(winWidth, winHeight, renderer);
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

void Menu::renderMoonParalax(int winWidth, int winHeight, IRenderer *renderer) {
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
    renderer->drawTextureEx(moonSky, static_cast<int>(parallaxOffsetSky), 0, winWidth, winHeight, 0.0, false, false);
    renderer->drawTextureEx(moonSky, static_cast<int>(parallaxOffsetSky - winWidth), 0, winWidth, winHeight, 0.0, false,
                            false);
  }

  if (moonBack != nullptr) {
    renderer->drawTextureEx(moonBack, static_cast<int>(parallaxOffsetBack), 0, winWidth, winHeight, 0.0, false, false);
    renderer->drawTextureEx(moonBack, static_cast<int>(parallaxOffsetBack - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (moonMid != nullptr) {
    renderer->drawTextureEx(moonMid, static_cast<int>(parallaxOffsetMid), 0, winWidth, winHeight, 0.0, false, false);
    renderer->drawTextureEx(moonMid, static_cast<int>(parallaxOffsetMid - winWidth), 0, winWidth, winHeight, 0.0, false,
                            false);
  }

  if (moonFront != nullptr) {
    renderer->drawTextureEx(moonFront, static_cast<int>(parallaxOffsetFront), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(moonFront, static_cast<int>(parallaxOffsetFront - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (moonFloor != nullptr) {
    renderer->drawTextureEx(moonFloor, static_cast<int>(parallaxOffsetFloor), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(moonFloor, static_cast<int>(parallaxOffsetFloor - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }
}