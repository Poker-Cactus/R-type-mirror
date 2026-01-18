/**
 * @file LobbyMenu.cpp
 * @brief Menu for creating and joining game lobbies.
 */

#include "LobbyMenu.hpp"
#include "../../../include/Settings.hpp"
#include "../../../interface/Geometry.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>

LobbyMenu::LobbyMenu(std::shared_ptr<IRenderer> renderer)
    : m_renderer(std::move(renderer)), m_font(nullptr), m_titleFont(nullptr), m_moonSky(nullptr), m_moonBack(nullptr),
      m_moonMid(nullptr), m_moonFront(nullptr), m_moonFloor(nullptr), m_parallaxOffsetSky(0.0F),
      m_parallaxOffsetBack(0.0F), m_parallaxOffsetMid(0.0F), m_parallaxOffsetFront(0.0F), m_parallaxOffsetFloor(0.0F),
      m_currentIndex(0), m_isEnteringCode(false), m_lobbyCodeInput(""), m_shouldEnterLobbyRoom(false),
      m_isCreatingLobby(false), m_isSolo(false), m_networkManager(nullptr)
{
}

LobbyMenu::~LobbyMenu()
{
  cleanup();
}

void LobbyMenu::init(Settings &settings)
{
  m_settings = &settings;
  try {
    constexpr int MENU_FONT_SIZE = 24;
    constexpr int TITLE_FONT_SIZE = 36;

    m_font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", MENU_FONT_SIZE);
    m_titleFont = m_renderer->loadFont("client/assets/font.opf/r-type.otf", TITLE_FONT_SIZE);

    // Load parallax background
    m_moonSky = m_renderer->loadTexture("client/assets/moon-para/moon_sky.png");
    m_moonBack = m_renderer->loadTexture("client/assets/moon-para/moon_back.png");
    m_moonMid = m_renderer->loadTexture("client/assets/moon-para/moon_mid.png");
    m_moonFront = m_renderer->loadTexture("client/assets/moon-para/moon_front.png");
    m_moonFloor = m_renderer->loadTexture("client/assets/moon-para/moon_floor.png");

  } catch (const std::exception &e) {
    std::cerr << "[LobbyMenu] Failed to load assets: " << e.what() << '\n';
  }
}

void LobbyMenu::cleanup()
{
  // Textures are managed by the renderer
  if (m_font != nullptr && m_renderer != nullptr)
    m_renderer->freeFont(m_font);
  if (m_titleFont != nullptr && m_renderer != nullptr)
    m_renderer->freeFont(m_titleFont);
  if (m_moonSky != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(m_moonSky);
  if (m_moonBack != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(m_moonBack);
  if (m_moonMid != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(m_moonMid);
  if (m_moonFront != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(m_moonFront);
  if (m_moonFloor != nullptr && m_renderer != nullptr)
    m_renderer->freeTexture(m_moonFloor);
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

void LobbyMenu::startDifficultySelection()
{
  m_isSelectingDifficulty = true;
  m_difficultyIndex = 1; // Default to Medium
  m_isSelectingMode = false;
}

void LobbyMenu::render(const WindowDimensions &windowDims)
{
  // Update highscore refresh timer
  float deltaTime = m_renderer->getDeltaTime();
  m_highscoreRefreshTimer += deltaTime;
  if (m_highscoreRefreshTimer >= HIGHSCORE_REFRESH_INTERVAL) {
    m_highscoreRefreshTimer = 0.0F;
    m_highscoreManager.refreshHighscores();
  }

  renderBackground(windowDims);

  // Draw title
  if (m_titleFont != nullptr) {
    const std::string title = "R TYPE";
    int titleWidth = 0;
    int titleHeight = 0;
    m_renderer->getTextSize(m_titleFont, title, titleWidth, titleHeight);

    const int titleX = (windowDims.width - titleWidth) / 2;
    constexpr int TITLE_Y_OFFSET = 80;
    const Color titleColor = {.r = 4, .g = 196, .b = 199, .a = 255};

    m_renderer->drawText(m_titleFont, title, titleX, TITLE_Y_OFFSET, titleColor);
  }

  // Draw player name
  if (m_font != nullptr && m_settings != nullptr && !m_settings->username.empty()) {
    const std::string playerText = "Playing as " + m_settings->username;
    int playerWidth = 0;
    int playerHeight = 0;
    m_renderer->getTextSize(m_font, playerText, playerWidth, playerHeight);

    const int playerX = (windowDims.width - playerWidth) / 2;
    constexpr int PLAYER_Y_OFFSET = 130;
    const Color playerColor = {.r = 255, .g = 255, .b = 255, .a = 255};

    m_renderer->drawText(m_font, playerText, playerX, PLAYER_Y_OFFSET, playerColor);
  }

  // Draw highscores
  renderHighscores(windowDims);

  if (m_isEnteringCode) {
    renderLobbyCodeInput(windowDims);
  } else if (m_isSelectingDifficulty) {
    renderDifficultySelection(windowDims);
  } else if (m_isSelectingMode) {
    renderModeSelection(windowDims);
  } else {
    renderMenuOptions(windowDims);
  }
}

void LobbyMenu::renderBackground(const WindowDimensions &windowDims)
{
  constexpr float SKY_SPEED = 5.0F;
  constexpr float BACK_SPEED = 15.0F;
  constexpr float MID_SPEED = 30.0F;
  constexpr float FRONT_SPEED = 50.0F;
  constexpr float FLOOR_SPEED = 70.0F;

  float deltaTime = m_renderer->getDeltaTime();

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
    m_renderer->drawTextureEx(m_moonSky, static_cast<int>(m_parallaxOffsetSky), 0, windowDims.width, windowDims.height,
                              0.0F, false, false);
    m_renderer->drawTextureEx(m_moonSky, static_cast<int>(m_parallaxOffsetSky - static_cast<float>(windowDims.width)),
                              0, windowDims.width, windowDims.height, 0.0F, false, false);
  }

  if (m_moonBack != nullptr) {
    m_renderer->drawTextureEx(m_moonBack, static_cast<int>(m_parallaxOffsetBack), 0, windowDims.width,
                              windowDims.height, 0.0F, false, false);
    m_renderer->drawTextureEx(m_moonBack, static_cast<int>(m_parallaxOffsetBack - static_cast<float>(windowDims.width)),
                              0, windowDims.width, windowDims.height, 0.0F, false, false);
  }

  if (m_moonMid != nullptr) {
    m_renderer->drawTextureEx(m_moonMid, static_cast<int>(m_parallaxOffsetMid), 0, windowDims.width, windowDims.height,
                              0.0F, false, false);
    m_renderer->drawTextureEx(m_moonMid, static_cast<int>(m_parallaxOffsetMid - static_cast<float>(windowDims.width)),
                              0, windowDims.width, windowDims.height, 0.0F, false, false);
  }

  if (m_moonFront != nullptr) {
    m_renderer->drawTextureEx(m_moonFront, static_cast<int>(m_parallaxOffsetFront), 0, windowDims.width,
                              windowDims.height, 0.0F, false, false);
    m_renderer->drawTextureEx(m_moonFront,
                              static_cast<int>(m_parallaxOffsetFront - static_cast<float>(windowDims.width)), 0,
                              windowDims.width, windowDims.height, 0.0F, false, false);
  }

  if (m_moonFloor != nullptr) {
    m_renderer->drawTextureEx(m_moonFloor, static_cast<int>(m_parallaxOffsetFloor), 0, windowDims.width,
                              windowDims.height, 0.0F, false, false);
    m_renderer->drawTextureEx(m_moonFloor,
                              static_cast<int>(m_parallaxOffsetFloor - static_cast<float>(windowDims.width)), 0,
                              windowDims.width, windowDims.height, 0.0F, false, false);
  }
}

void LobbyMenu::renderMenuOptions(const WindowDimensions &windowDims)
{
  if (m_font == nullptr) {
    return;
  }

  constexpr int ITEM_SPACING = 60;
  const int startY = (windowDims.height - 120) / 2;

  for (std::size_t i = 0; i < m_menuItems.size(); ++i) {
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(m_font, m_menuItems[i], textWidth, textHeight);

    const int pos_x = (windowDims.width - textWidth) / 2;
    const int pos_y = startY + (static_cast<int>(i) * ITEM_SPACING);

    // Highlight selected item
    const Color color = (i == m_currentIndex) ? Color{.r = 4, .g = 196, .b = 199, .a = 255}
                                              : Color{.r = 255, .g = 255, .b = 255, .a = 255};

    m_renderer->drawText(m_font, m_menuItems[i], pos_x, pos_y, color);
  }

  // Draw instructions at bottom
  const std::string instructions = "Use UP/DOWN to navigate, ENTER to select";
  int instrWidth = 0;
  int instrHeight = 0;
  m_renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (windowDims.width - instrWidth) / 2;
  const int instrY = windowDims.height - 60;
  const Color instrColor = {.r = 150, .g = 150, .b = 150, .a = 255};

  m_renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::renderLobbyCodeInput(const WindowDimensions &windowDims)
{
  if (m_font == nullptr) {
    return;
  }

  // Draw prompt
  const std::string prompt = "Enter Lobby Code:";
  int promptWidth = 0;
  int promptHeight = 0;
  m_renderer->getTextSize(m_font, prompt, promptWidth, promptHeight);

  const int promptX = (windowDims.width - promptWidth) / 2;
  const int promptY = ((windowDims.height - promptHeight) / 2) - 60;
  const Color promptColor = {.r = 255, .g = 255, .b = 255, .a = 255};

  m_renderer->drawText(m_font, prompt, promptX, promptY, promptColor);

  // Draw input box
  const std::string displayCode = m_lobbyCodeInput + "_";
  int codeWidth = 0;
  int codeHeight = 0;
  m_renderer->getTextSize(m_font, displayCode, codeWidth, codeHeight);

  const int codeX = (windowDims.width - codeWidth) / 2;
  const int codeY = (windowDims.height - codeHeight) / 2;
  const Color codeColor = {.r = 4, .g = 196, .b = 199, .a = 255};

  m_renderer->drawText(m_font, displayCode, codeX, codeY, codeColor);

  // Draw instructions
  const std::string instructions = "Type code and press ENTER, TAB to cancel";
  int instrWidth = 0;
  int instrHeight = 0;
  m_renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (windowDims.width - instrWidth) / 2;
  const int instrY = ((windowDims.height - instrHeight) / 2) + 80;
  const Color instrColor = {.r = 150, .g = 150, .b = 150, .a = 255};

  m_renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::renderDifficultySelection(const WindowDimensions &windowDims)
{
  if (m_font == nullptr) {
    return;
  }

  // Draw prompt
  const std::string prompt = "Select Difficulty:";
  int promptWidth = 0;
  int promptHeight = 0;
  m_renderer->getTextSize(m_font, prompt, promptWidth, promptHeight);

  const int promptX = (windowDims.width - promptWidth) / 2;
  const int promptY = ((windowDims.height - promptHeight) / 2) - 100;
  const Color promptColor = {.r = 255, .g = 255, .b = 255, .a = 255};

  m_renderer->drawText(m_font, prompt, promptX, promptY, promptColor);

  // Render difficulty options
  constexpr int ITEM_SPACING = 50;
  const int startY = ((windowDims.height - promptHeight) / 2) - 20;

  for (std::size_t i = 0; i < m_difficultyItems.size(); ++i) {
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(m_font, m_difficultyItems[i], textWidth, textHeight);

    const int pos_x = (windowDims.width - textWidth) / 2;
    const int pos_y = startY + (static_cast<int>(i) * ITEM_SPACING);

    // Highlight selected item
    const Color color = (i == m_difficultyIndex) ? Color{.r = 4, .g = 196, .b = 199, .a = 255}
                                                 : Color{.r = 255, .g = 255, .b = 255, .a = 255};

    m_renderer->drawText(m_font, m_difficultyItems[i], pos_x, pos_y, color);

    // Draw checkmark if this is the currently selected difficulty (persisted)
    // Actually, we just highlight the current selection in the navigation.
    // The "selected" one is just the one we create with.
  }

  // Draw instructions
  const std::string instructions =
    m_isSolo ? "ENTER to Start, BACKSPACE to Go Back" : "ENTER to Create, BACKSPACE to Cancel";
  int instrWidth = 0;
  int instrHeight = 0;
  m_renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (windowDims.width - instrWidth) / 2;
  const int instrY = windowDims.height - 80;
  const Color instrColor = {.r = 150, .g = 150, .b = 150, .a = 255};

  m_renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::renderModeSelection(const WindowDimensions &windowDims)
{
  if (m_font == nullptr) {
    return;
  }

  const std::string prompt = "Select Game Mode";
  int promptWidth = 0;
  int promptHeight = 0;
  m_renderer->getTextSize(m_font, prompt, promptWidth, promptHeight);

  const int promptX = (windowDims.width - promptWidth) / 2;
  const int promptY = ((windowDims.height - promptHeight) / 2) - 100;
  const Color promptColor = {.r = 255, .g = 255, .b = 255, .a = 255};

  m_renderer->drawText(m_font, prompt, promptX, promptY, promptColor);

  constexpr int ITEM_SPACING = 50;
  const int startY = ((windowDims.height - promptHeight) / 2) - 20;

  for (std::size_t i = 0; i < m_modeItems.size(); ++i) {
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(m_font, m_modeItems[i], textWidth, textHeight);

    const int pos_x = (windowDims.width - textWidth) / 2;
    const int pos_y = startY + (static_cast<int>(i) * ITEM_SPACING);

    const Color color =
      (i == m_modeIndex) ? Color{.r = 4, .g = 196, .b = 199, .a = 255} : Color{.r = 255, .g = 255, .b = 255, .a = 255};

    m_renderer->drawText(m_font, m_modeItems[i], pos_x, pos_y, color);
  }

  const std::string instructions =
    m_isSolo ? "ENTER to Start, BACKSPACE to Go Back" : "ENTER to Create, BACKSPACE to Go Back";
  int instrWidth = 0;
  int instrHeight = 0;
  m_renderer->getTextSize(m_font, instructions, instrWidth, instrHeight);

  const int instrX = (windowDims.width - instrWidth) / 2;
  const int instrY = windowDims.height - 80;
  const Color instrColor = {.r = 150, .g = 150, .b = 150, .a = 255};

  m_renderer->drawText(m_font, instructions, instrX, instrY, instrColor);
}

void LobbyMenu::process(MenuState *currentState, Settings &settings)
{
  if (m_isEnteringCode) {
    handleTextInput();
  } else if (m_isSelectingDifficulty) {
    handleDifficultyNavigation();
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      selectDifficultyOption();
    }
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
      if (m_isSolo) {
        *currentState = MenuState::AI_DIFFICULTY;
      }
      m_isSelectingDifficulty = false;
    }
  } else if (m_isSelectingMode) {
    handleModeNavigation();
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      selectModeOption();
    }
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
      m_isSelectingMode = false;
      m_isSelectingDifficulty = true;
    }
  } else {
    handleMenuNavigation(settings);

    if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      selectCurrentOption(currentState);
    }

    if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
      *currentState = MenuState::MAIN_MENU;
    }
  }
}

void LobbyMenu::handleMenuNavigation(Settings &settings)
{
  if (m_renderer->isKeyJustPressed(settings.down)) {
    m_currentIndex = (m_currentIndex + 1) % m_menuItems.size();
  }

  if (m_renderer->isKeyJustPressed(settings.up)) {
    m_currentIndex = (m_currentIndex - 1 + m_menuItems.size()) % m_menuItems.size();
  }
}

void LobbyMenu::handleDifficultyNavigation()
{
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    m_difficultyIndex = (m_difficultyIndex + 1) % m_difficultyItems.size();
  }

  if (m_renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    m_difficultyIndex = (m_difficultyIndex - 1 + m_difficultyItems.size()) % m_difficultyItems.size();
  }
}

void LobbyMenu::handleModeNavigation()
{
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    m_modeIndex = (m_modeIndex + 1) % m_modeItems.size();
  }

  if (m_renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    m_modeIndex = (m_modeIndex - 1 + m_modeItems.size()) % m_modeItems.size();
  }
}

void LobbyMenu::handleTextInput()
{
  // Cancel with TAB - go back to menu options
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_TAB)) {
    m_isEnteringCode = false;
    m_lobbyCodeInput.clear();
    return;
  }

  // Backspace to delete characters
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE) && !m_lobbyCodeInput.empty()) {
    m_lobbyCodeInput.pop_back();
    return;
  }

  // Confirm with ENTER
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
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
      if (m_renderer->isKeyJustPressed(key)) {
        char chr = static_cast<char>('A' + (key - KeyCode::KEY_A));
        m_lobbyCodeInput += chr;
        return;
      }
    }

    // Check 0-9
    for (int key = KeyCode::KEY_0; key <= KeyCode::KEY_9; ++key) {
      if (m_renderer->isKeyJustPressed(key)) {
        char chr = static_cast<char>('0' + (key - KeyCode::KEY_0));
        m_lobbyCodeInput += chr;
        return;
      }
    }
  }
}

void LobbyMenu::renderHighscores(const WindowDimensions &windowDims)
{
  if (m_font == nullptr) {
    return;
  }

  const auto &highscores = m_highscoreManager.getHighscores();
  if (highscores.empty()) {
    return;
  }

  // Draw "Highscores" title
  const std::string title = "HIGHSCORES";
  int titleWidth = 0;
  int titleHeight = 0;
  m_renderer->getTextSize(m_font, title, titleWidth, titleHeight);

  const int titleX = (windowDims.width - titleWidth) / 2;
  constexpr int HIGHSCORE_TITLE_Y = 170;
  const Color titleColor = {.r = 4, .g = 196, .b = 199, .a = 255};
  m_renderer->drawText(m_font, title, titleX, HIGHSCORE_TITLE_Y, titleColor);

  // Draw top 5 highscores
  constexpr int HIGHSCORE_START_Y = 200;
  constexpr int HIGHSCORE_SPACING = 25;
  const Color scoreColor = {.r = 255, .g = 255, .b = 255, .a = 255};

  for (std::size_t i = 0; i < highscores.size() && i < 5; ++i) {
    const auto &entry = highscores[i];
    std::string difficultyStr;
    switch (entry.difficulty) {
    case Difficulty::EASY:
      difficultyStr = "Easy";
      break;
    case Difficulty::MEDIUM:
      difficultyStr = "Medium";
      break;
    case Difficulty::EXPERT:
      difficultyStr = "Expert";
      break;
    }

    const std::string scoreText =
      std::to_string(i + 1) + ". " + entry.username + "  " + std::to_string(entry.score) + "  " + difficultyStr;

    int scoreWidth = 0;
    int scoreHeight = 0;
    m_renderer->getTextSize(m_font, scoreText, scoreWidth, scoreHeight);

    const int scoreX = (windowDims.width - scoreWidth) / 2;
    const int scoreY = HIGHSCORE_START_Y + (static_cast<int>(i) * HIGHSCORE_SPACING);

    m_renderer->drawText(m_font, scoreText, scoreX, scoreY, scoreColor);
  }
}

void LobbyMenu::refreshHighscores()
{
  m_highscoreManager.refreshHighscores();
  m_highscoreRefreshTimer = 0.0F; // Reset timer after manual refresh
}

void LobbyMenu::selectCurrentOption(MenuState *currentState)
{
  switch (m_currentIndex) {
  case 0: // Create Lobby
    std::cout << "[LobbyMenu] Select difficulty for new lobby" << '\n';
    m_isSelectingDifficulty = true;
    m_difficultyIndex = 1; // Reset to Medium
    m_isSelectingMode = false;
    break;

  case 1: // Join Lobby
    std::cout << "[LobbyMenu] Entering lobby code input mode" << '\n';
    m_isEnteringCode = true;
    m_lobbyCodeInput.clear();
    break;

  case 2: // Solo
    std::cout << "[LobbyMenu] Going to AI difficulty selection" << '\n';
    *currentState = MenuState::AI_DIFFICULTY;
    break;

  case 3: // Clear Highscores
    std::cout << "[LobbyMenu] Clearing highscores" << '\n';
    m_highscoreManager.clearHighscores();
    break;

  case 4: // Back
    *currentState = MenuState::MAIN_MENU;
    break;

  default:
    break;
  }
}

void LobbyMenu::selectDifficultyOption()
{
  switch (m_difficultyIndex) {
  case 0:
    m_selectedDifficulty = Difficulty::EASY;
    break;
  case 1:
    m_selectedDifficulty = Difficulty::MEDIUM;
    break;
  case 2:
    m_selectedDifficulty = Difficulty::EXPERT;
    break;
  }

  std::cout << "[LobbyMenu] Selected difficulty: "
            << (m_selectedDifficulty == Difficulty::EASY
                  ? "EASY"
                  : (m_selectedDifficulty == Difficulty::MEDIUM ? "MEDIUM" : "EXPERT"))
            << " (value: " << static_cast<int>(m_selectedDifficulty) << ")" << '\n';

  m_isSelectingDifficulty = false;
  m_isSelectingMode = true;
  m_modeIndex = 0; // Default to Classic
}

void LobbyMenu::selectModeOption()
{
  switch (m_modeIndex) {
  case 0:
    m_selectedGameMode = GameMode::CLASSIC;
    break;
  case 1:
    m_selectedGameMode = GameMode::ENDLESS;
    break;
  }

  std::cout << "[LobbyMenu] Creating new lobby with mode: "
            << (m_selectedGameMode == GameMode::CLASSIC ? "CLASSIC" : "ENDLESS") << '\n';

  m_isCreatingLobby = true;
  m_lobbyCodeInput.clear();
  m_shouldEnterLobbyRoom = true;
  m_isSelectingMode = false;
}
