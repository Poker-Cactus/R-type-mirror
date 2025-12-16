/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyMenu - Menu for creating and joining game lobbies
*/

#include "LobbyMenu.hpp"
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
    std::cerr << "[LobbyMenu] Failed to load assets: " << e.what() << std::endl;
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

void LobbyMenu::render(int winWidth, int winHeight, IRenderer *renderer)
{
  renderBackground(winWidth, winHeight, renderer);

  // Draw title
  if (m_titleFont != nullptr) {
    const std::string title = "MULTIPLAYER";
    int titleWidth = 0;
    int titleHeight = 0;
    renderer->getTextSize(m_titleFont, title, titleWidth, titleHeight);

    const int titleX = (winWidth - titleWidth) / 2;
    constexpr int TITLE_Y_OFFSET = 80;
    const Color titleColor = {4, 196, 199, 255};

    renderer->drawText(m_titleFont, title, titleX, TITLE_Y_OFFSET, titleColor);
  }

  if (m_isEnteringCode) {
    renderLobbyCodeInput(winWidth, winHeight, renderer);
  } else {
    renderMenuOptions(winWidth, winHeight, renderer);
  }
}

void LobbyMenu::renderBackground(int winWidth, int winHeight, IRenderer *renderer)
{
  float deltaTime = renderer->getDeltaTime();

  // Update parallax offsets
  m_parallaxOffsetSky += deltaTime * 5.0f;
  m_parallaxOffsetBack += deltaTime * 15.0f;
  m_parallaxOffsetMid += deltaTime * 30.0f;
  m_parallaxOffsetFront += deltaTime * 50.0f;
  m_parallaxOffsetFloor += deltaTime * 70.0f;

  // Reset offsets when they exceed window width
  if (m_parallaxOffsetSky >= static_cast<float>(winWidth)) {
    m_parallaxOffsetSky = 0.0f;
  }
  if (m_parallaxOffsetBack >= static_cast<float>(winWidth)) {
    m_parallaxOffsetBack = 0.0f;
  }
  if (m_parallaxOffsetMid >= static_cast<float>(winWidth)) {
    m_parallaxOffsetMid = 0.0f;
  }
  if (m_parallaxOffsetFront >= static_cast<float>(winWidth)) {
    m_parallaxOffsetFront = 0.0f;
  }
  if (m_parallaxOffsetFloor >= static_cast<float>(winWidth)) {
    m_parallaxOffsetFloor = 0.0f;
  }

  // Draw parallax layers
  if (m_moonSky != nullptr) {
    renderer->drawTextureEx(m_moonSky, static_cast<int>(m_parallaxOffsetSky), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(m_moonSky, static_cast<int>(m_parallaxOffsetSky - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (m_moonBack != nullptr) {
    renderer->drawTextureEx(m_moonBack, static_cast<int>(m_parallaxOffsetBack), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(m_moonBack, static_cast<int>(m_parallaxOffsetBack - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (m_moonMid != nullptr) {
    renderer->drawTextureEx(m_moonMid, static_cast<int>(m_parallaxOffsetMid), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(m_moonMid, static_cast<int>(m_parallaxOffsetMid - winWidth), 0, winWidth, winHeight, 0.0,
                            false, false);
  }

  if (m_moonFront != nullptr) {
    renderer->drawTextureEx(m_moonFront, static_cast<int>(m_parallaxOffsetFront), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(m_moonFront, static_cast<int>(m_parallaxOffsetFront - winWidth), 0, winWidth, winHeight,
                            0.0, false, false);
  }

  if (m_moonFloor != nullptr) {
    renderer->drawTextureEx(m_moonFloor, static_cast<int>(m_parallaxOffsetFloor), 0, winWidth, winHeight, 0.0, false,
                            false);
    renderer->drawTextureEx(m_moonFloor, static_cast<int>(m_parallaxOffsetFloor - winWidth), 0, winWidth, winHeight,
                            0.0, false, false);
  }
}

void LobbyMenu::renderMenuOptions(int winWidth, int winHeight, IRenderer *renderer)
{
  if (m_font == nullptr) {
    return;
  }

  constexpr int ITEM_SPACING = 60;
  const int startY = winHeight / 2 - 60;

  for (std::size_t i = 0; i < m_menuItems.size(); ++i) {
    int textWidth = 0;
    int textHeight = 0;
    renderer->getTextSize(m_font, m_menuItems[i], textWidth, textHeight);

    const int x = (winWidth - textWidth) / 2;
    const int y = startY + static_cast<int>(i) * ITEM_SPACING;

    // Highlight selected item
    const Color color = (i == m_currentIndex) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};

    renderer->drawText(m_font, m_menuItems[i], x, y, color);
  }

  // Draw instructions at bottom
  const std::string instructions = "Use UP/DOWN to navigate, ENTER to select";
  int instrWidth = 0;
  int instrHeight = 0;
  renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (winWidth - instrWidth) / 2;
  const int instrY = winHeight - 60;
  const Color instrColor = {150, 150, 150, 255};

  renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::renderLobbyCodeInput(int winWidth, int winHeight, IRenderer *renderer)
{
  if (m_font == nullptr) {
    return;
  }

  // Draw prompt
  const std::string prompt = "Enter Lobby Code:";
  int promptWidth = 0;
  int promptHeight = 0;
  renderer->getTextSize(m_font, prompt, promptWidth, promptHeight);

  const int promptX = (winWidth - promptWidth) / 2;
  const int promptY = winHeight / 2 - 60;
  const Color promptColor = {255, 255, 255, 255};

  renderer->drawText(m_font, prompt, promptX, promptY, promptColor);

  // Draw input box
  const std::string displayCode = m_lobbyCodeInput + "_";
  int codeWidth = 0;
  int codeHeight = 0;
  renderer->getTextSize(m_font, displayCode, codeWidth, codeHeight);

  const int codeX = (winWidth - codeWidth) / 2;
  const int codeY = winHeight / 2;
  const Color codeColor = {4, 196, 199, 255};

  renderer->drawText(m_font, displayCode, codeX, codeY, codeColor);

  // Draw instructions
  const std::string instructions = "Type code and press ENTER, TAB to cancel";
  int instrWidth = 0;
  int instrHeight = 0;
  renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (winWidth - instrWidth) / 2;
  const int instrY = winHeight / 2 + 80;
  const Color instrColor = {150, 150, 150, 255};

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
      std::cout << "[LobbyMenu] Joining lobby with code: " << m_lobbyCodeInput << std::endl;
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
        char c = static_cast<char>('A' + (key - KeyCode::KEY_A));
        m_lobbyCodeInput += c;
        return;
      }
    }

    // Check 0-9
    for (int key = KeyCode::KEY_0; key <= KeyCode::KEY_9; ++key) {
      if (renderer->isKeyJustPressed(key)) {
        char c = static_cast<char>('0' + (key - KeyCode::KEY_0));
        m_lobbyCodeInput += c;
        return;
      }
    }
  }
}

void LobbyMenu::selectCurrentOption(MenuState *currentState)
{
  switch (m_currentIndex) {
  case 0: // Create Lobby
    std::cout << "[LobbyMenu] Creating new lobby" << std::endl;
    m_isCreatingLobby = true;
    m_lobbyCodeInput.clear();
    m_shouldEnterLobbyRoom = true;
    break;

  case 1: // Join Lobby
    std::cout << "[LobbyMenu] Entering lobby code input mode" << std::endl;
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
