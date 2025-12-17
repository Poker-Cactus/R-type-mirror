/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyMenu - Menu for creating and joining game lobbies
*/

#include "LobbyMenu.hpp"
#include "../../../interface/Geometry.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>

LobbyMenu::~LobbyMenu()
{
  cleanup();
}

void LobbyMenu::init(IRenderer *renderer)
{
  try {
    constexpr int MENU_FONT_SIZE = 24;
    constexpr int TITLE_FONT_SIZE = 36;

    m_font = renderer->loadFont("client/assets/font.opf/r-type.otf", MENU_FONT_SIZE);
    m_titleFont = renderer->loadFont("client/assets/font.opf/r-type.otf", TITLE_FONT_SIZE);

    // Load parallax background
    m_moonSky = renderer->loadTexture("client/assets/moon-para/moon_sky.png");
    m_moonBack = renderer->loadTexture("client/assets/moon-para/moon_back.png");
    m_moonMid = renderer->loadTexture("client/assets/moon-para/moon_mid.png");
    m_moonFront = renderer->loadTexture("client/assets/moon-para/moon_front.png");
    m_moonFloor = renderer->loadTexture("client/assets/moon-para/moon_floor.png");

  } catch (const std::exception &e) {
    std::cerr << "[LobbyMenu] Failed to load assets: " << e.what() << '\n';
  }
}

void LobbyMenu::cleanup()
{
  // Textures are managed by the renderer
  m_font = nullptr;
  m_titleFont = nullptr;
  m_moonSky = nullptr;
  m_moonBack = nullptr;
  m_moonMid = nullptr;
  m_moonFront = nullptr;
  m_moonFloor = nullptr;
}

void LobbyMenu::setNetworkManager(std::shared_ptr<INetworkManager> networkManager)
{
  m_networkManager = std::move(networkManager);
}

void LobbyMenu::render(const WindowDimensions &windowDims, IRenderer *renderer)
{
  renderBackground(windowDims, renderer);

  // Draw title
  if (m_titleFont != nullptr) {
    const std::string title = "MULTIPLAYER";
    int titleWidth = 0;
    int titleHeight = 0;
    renderer->getTextSize(m_titleFont, title, titleWidth, titleHeight);

    const int titleX = (windowDims.width - titleWidth) / 2;
    constexpr int TITLE_Y_OFFSET = 80;
    const Color titleColor = {.r = 4, .g = 196, .b = 199, .a = 255};

    renderer->drawText(m_titleFont, title, titleX, TITLE_Y_OFFSET, titleColor);
  }

  if (m_isEnteringCode) {
    renderLobbyCodeInput(windowDims, renderer);
  } else {
    renderMenuOptions(windowDims, renderer);
  }
}

void LobbyMenu::renderBackground(const WindowDimensions &windowDims, IRenderer *renderer)
{
  constexpr float SKY_SPEED = 5.0F;
  constexpr float BACK_SPEED = 15.0F;
  constexpr float MID_SPEED = 30.0F;
  constexpr float FRONT_SPEED = 50.0F;
  constexpr float FLOOR_SPEED = 70.0F;

  float deltaTime = renderer->getDeltaTime();

  // Update parallax offsets
  m_parallaxOffsetSky += deltaTime * SKY_SPEED;
  m_parallaxOffsetBack += deltaTime * BACK_SPEED;
  m_parallaxOffsetMid += deltaTime * MID_SPEED;
  m_parallaxOffsetFront += deltaTime * FRONT_SPEED;
  m_parallaxOffsetFloor += deltaTime * FLOOR_SPEED;

  // Reset offsets when they exceed window width
  if (m_parallaxOffsetSky >= static_cast<float>(windowDims.width)) {
    m_parallaxOffsetSky = 0.0F;
  }
  if (m_parallaxOffsetBack >= static_cast<float>(windowDims.width)) {
    m_parallaxOffsetBack = 0.0F;
  }
  if (m_parallaxOffsetMid >= static_cast<float>(windowDims.width)) {
    m_parallaxOffsetMid = 0.0F;
  }
  if (m_parallaxOffsetFront >= static_cast<float>(windowDims.width)) {
    m_parallaxOffsetFront = 0.0F;
  }
  if (m_parallaxOffsetFloor >= static_cast<float>(windowDims.width)) {
    m_parallaxOffsetFloor = 0.0F;
  }

  // Draw parallax layers
  if (m_moonSky != nullptr) {
    renderer->drawTextureEx(
      m_moonSky,
      {.x = static_cast<int>(m_parallaxOffsetSky), .y = 0, .width = windowDims.width, .height = windowDims.height},
      0.0F, false, false);
    renderer->drawTextureEx(m_moonSky,
                            {.x = static_cast<int>(m_parallaxOffsetSky - static_cast<float>(windowDims.width)),
                             .y = 0,
                             .width = windowDims.width,
                             .height = windowDims.height},
                            0.0F, false, false);
  }

  if (m_moonBack != nullptr) {
    renderer->drawTextureEx(
      m_moonBack,
      {.x = static_cast<int>(m_parallaxOffsetBack), .y = 0, .width = windowDims.width, .height = windowDims.height},
      0.0F, false, false);
    renderer->drawTextureEx(m_moonBack,
                            {.x = static_cast<int>(m_parallaxOffsetBack - static_cast<float>(windowDims.width)),
                             .y = 0,
                             .width = windowDims.width,
                             .height = windowDims.height},
                            0.0F, false, false);
  }

  if (m_moonMid != nullptr) {
    renderer->drawTextureEx(
      m_moonMid,
      {.x = static_cast<int>(m_parallaxOffsetMid), .y = 0, .width = windowDims.width, .height = windowDims.height},
      0.0F, false, false);
    renderer->drawTextureEx(m_moonMid,
                            {.x = static_cast<int>(m_parallaxOffsetMid - static_cast<float>(windowDims.width)),
                             .y = 0,
                             .width = windowDims.width,
                             .height = windowDims.height},
                            0.0F, false, false);
  }

  if (m_moonFront != nullptr) {
    renderer->drawTextureEx(
      m_moonFront,
      {.x = static_cast<int>(m_parallaxOffsetFront), .y = 0, .width = windowDims.width, .height = windowDims.height},
      0.0F, false, false);
    renderer->drawTextureEx(m_moonFront,
                            {.x = static_cast<int>(m_parallaxOffsetFront - static_cast<float>(windowDims.width)),
                             .y = 0,
                             .width = windowDims.width,
                             .height = windowDims.height},
                            0.0F, false, false);
  }

  if (m_moonFloor != nullptr) {
    renderer->drawTextureEx(
      m_moonFloor,
      {.x = static_cast<int>(m_parallaxOffsetFloor), .y = 0, .width = windowDims.width, .height = windowDims.height},
      0.0F, false, false);
    renderer->drawTextureEx(m_moonFloor,
                            {.x = static_cast<int>(m_parallaxOffsetFloor - static_cast<float>(windowDims.width)),
                             .y = 0,
                             .width = windowDims.width,
                             .height = windowDims.height},
                            0.0F, false, false);
  }
}

void LobbyMenu::renderMenuOptions(const WindowDimensions &windowDims, IRenderer *renderer)
{
  if (m_font == nullptr) {
    return;
  }

  constexpr int ITEM_SPACING = 60;
  const int startY = (windowDims.height - 120) / 2;

  for (std::size_t i = 0; i < m_menuItems.size(); ++i) {
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(m_font, m_menuItems[i], textWidth, textHeight);

    const int pos_x = (windowDims.width - textWidth) / 2;
    const int pos_y = startY + (static_cast<int>(i) * ITEM_SPACING);

    // Highlight selected item
    const Color color = (i == m_currentIndex) ? Color{.r = 4, .g = 196, .b = 199, .a = 255}
                                              : Color{.r = 255, .g = 255, .b = 255, .a = 255};

    renderer->drawText(m_font, m_menuItems[i], pos_x, pos_y, color);
  }

  // Draw instructions at bottom
  const std::string instructions = "Use UP/DOWN to navigate, ENTER to select";
  int instrWidth = 0;
  int instrHeight = 0;
  renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (windowDims.width - instrWidth) / 2;
  const int instrY = windowDims.height - 60;
  const Color instrColor = {.r = 150, .g = 150, .b = 150, .a = 255};

  renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::renderLobbyCodeInput(const WindowDimensions &windowDims, IRenderer *renderer)
{
  if (m_font == nullptr) {
    return;
  }

  // Draw prompt
  const std::string prompt = "Enter Lobby Code:";
  int promptWidth = 0;
  int promptHeight = 0;
  renderer->getTextSize(m_font, prompt, promptWidth, promptHeight);

  const int promptX = (windowDims.width - promptWidth) / 2;
  const int promptY = ((windowDims.height - promptHeight) / 2) - 60;
  const Color promptColor = {.r = 255, .g = 255, .b = 255, .a = 255};

  renderer->drawText(m_font, prompt, promptX, promptY, promptColor);

  // Draw input box
  const std::string displayCode = m_lobbyCodeInput + "_";
  int codeWidth = 0;
  int codeHeight = 0;
  renderer->getTextSize(m_font, displayCode, codeWidth, codeHeight);

  const int codeX = (windowDims.width - codeWidth) / 2;
  const int codeY = (windowDims.height - codeHeight) / 2;
  const Color codeColor = {.r = 4, .g = 196, .b = 199, .a = 255};

  renderer->drawText(m_font, displayCode, codeX, codeY, codeColor);

  // Draw instructions
  const std::string instructions = "Type code and press ENTER, TAB to cancel";
  int instrWidth = 0;
  int instrHeight = 0;
  renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (windowDims.width - instrWidth) / 2;
  const int instrY = ((windowDims.height - instrHeight) / 2) + 80;
  const Color instrColor = {.r = 150, .g = 150, .b = 150, .a = 255};

  renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::process(IRenderer *renderer, MenuState *currentState)
{
  if (m_isEnteringCode) {
    handleTextInput(renderer);
  } else {
    handleMenuNavigation(renderer);

    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      selectCurrentOption(currentState);
    }

    if (renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
      *currentState = MenuState::MAIN_MENU;
    }
  }
}

void LobbyMenu::handleMenuNavigation(IRenderer *renderer)
{
  if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    m_currentIndex = (m_currentIndex + 1) % m_menuItems.size();
  }

  if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    m_currentIndex = (m_currentIndex - 1 + m_menuItems.size()) % m_menuItems.size();
  }
}

void LobbyMenu::handleTextInput(IRenderer *renderer)
{
  // Cancel with TAB - go back to menu options
  if (renderer->isKeyJustPressed(KeyCode::KEY_TAB)) {
    m_isEnteringCode = false;
    m_lobbyCodeInput.clear();
    return;
  }

  // Backspace to delete characters
  if (renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE) && !m_lobbyCodeInput.empty()) {
    m_lobbyCodeInput.pop_back();
    return;
  }

  // Confirm with ENTER
  if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    if (!m_lobbyCodeInput.empty()) {
      std::cout << "[LobbyMenu] Joining lobby with code: " << m_lobbyCodeInput << '\n';
      m_isCreatingLobby = false;
      m_shouldEnterLobbyRoom = true;
      m_isEnteringCode = false;
    }
    return;
  }

  // Letter/number input
  if (m_lobbyCodeInput.size() < MAX_LOBBY_CODE_LENGTH) {
    // Check A-Z
    for (int key = KeyCode::KEY_A; key <= KeyCode::KEY_Z; ++key) {
      if (renderer->isKeyJustPressed(key)) {
        char chr = static_cast<char>('A' + (key - KeyCode::KEY_A));
        m_lobbyCodeInput += chr;
        return;
      }
    }

    // Check 0-9
    for (int key = KeyCode::KEY_0; key <= KeyCode::KEY_9; ++key) {
      if (renderer->isKeyJustPressed(key)) {
        char chr = static_cast<char>('0' + (key - KeyCode::KEY_0));
        m_lobbyCodeInput += chr;
        return;
      }
    }
  }
}

void LobbyMenu::selectCurrentOption(MenuState *currentState)
{
  switch (m_currentIndex) {
  case 0: // Create Lobby
    std::cout << "[LobbyMenu] Creating new lobby" << '\n';
    m_isCreatingLobby = true;
    m_lobbyCodeInput.clear();
    m_shouldEnterLobbyRoom = true;
    break;

  case 1: // Join Lobby
    std::cout << "[LobbyMenu] Entering lobby code input mode" << '\n';
    m_isEnteringCode = true;
    m_lobbyCodeInput.clear();
    break;

  case 2: // Back
    *currentState = MenuState::MAIN_MENU;
    break;

  default:
    break;
  }
}
