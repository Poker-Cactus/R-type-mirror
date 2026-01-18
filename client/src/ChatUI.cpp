/**
 * @file ChatUI.cpp
 * @brief In-game chat interface implementation.
 */

#include "ChatUI.hpp"
#include "../interface/KeyCodes.hpp"
#include <algorithm>
#include <iostream>

ChatUI::ChatUI(std::shared_ptr<IRenderer> renderer) : m_renderer(std::move(renderer)) {}

bool ChatUI::init()
{
  if (!m_renderer) {
    std::cerr << "[ChatUI] Renderer is null!" << std::endl;
    return false;
  }

  m_font = m_renderer->loadFont("client/assets/font.opf/game.ttf", 16);
  m_smallFont = m_renderer->loadFont("client/assets/font.opf/game.ttf", 14);

  if (!m_font) {
    std::cerr << "[ChatUI] Failed to load chat font - continuing without chat" << std::endl;
    return false;
  }

  std::cout << "[ChatUI] Initialized successfully" << std::endl;
  return true;
}

void ChatUI::toggle()
{
  m_isVisible = !m_isVisible;
  m_isInputFocused = m_isVisible;
  if (!m_isVisible) {
    m_inputText.clear();
  }
  std::cout << "[ChatUI] Chat " << (m_isVisible ? "opened" : "closed") << std::endl;
}

void ChatUI::open()
{
  if (!m_isVisible) {
    m_isVisible = true;
    m_isInputFocused = true;
    std::cout << "[ChatUI] Chat opened" << std::endl;
  }
}

void ChatUI::close()
{
  if (m_isVisible) {
    m_isVisible = false;
    m_isInputFocused = false;
    m_inputText.clear();
    std::cout << "[ChatUI] Chat closed" << std::endl;
  }
}

void ChatUI::processInput()
{
  if (!m_isVisible || !m_renderer) {
    return;
  }
  if (m_isInputFocused) {
    handleTextInput();
    if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
      if (!m_inputText.empty()) {
        m_messageReady = true;
      }
    }
  }
}

void ChatUI::handleTextInput()
{
  if (!m_renderer) {
    return;
  }

  if (m_renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE) && !m_inputText.empty()) {
    m_inputText.pop_back();
    return;
  }

  if (m_inputText.size() >= MAX_INPUT_LENGTH) {
    return;
  }

  bool shiftPressed = m_renderer->isKeyPressed(KeyCode::KEY_LSHIFT) || m_renderer->isKeyPressed(KeyCode::KEY_RSHIFT);

  // Letters A-Z (lowercase by default)
  for (int key = KeyCode::KEY_A; key <= KeyCode::KEY_Z; key++) {
    if (m_renderer->isKeyJustPressed(key)) {
      char chr = static_cast<char>('a' + (key - KeyCode::KEY_A));
      if (shiftPressed) {
        chr = static_cast<char>('A' + (key - KeyCode::KEY_A));
      }
      m_inputText += chr;
      return;
    }
  }

  if (m_renderer->isKeyJustPressed(KeyCode::KEY_0)) {
    if (shiftPressed) {
      m_inputText += '0';
    } else {
      m_inputText += " ";
    }
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_1)) {
    m_inputText += shiftPressed ? '1' : ' ';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_2)) {
    if (shiftPressed) {
      m_inputText += '2';
    } else {
      m_inputText += " ";
    }
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_3)) {
    m_inputText += shiftPressed ? '3' : '"';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_4)) {
    m_inputText += shiftPressed ? '4' : '\'';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_5)) {
    m_inputText += shiftPressed ? '5' : '(';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_6)) {
    m_inputText += shiftPressed ? '6' : '-';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_7)) {
    if (shiftPressed) {
      m_inputText += '7';
    } else {
      m_inputText += " ";
    }
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_8)) {
    m_inputText += shiftPressed ? '8' : '_';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_9)) {
    if (shiftPressed) {
      m_inputText += '9';
    } else {
      m_inputText += " ";
    }
    return;
  }

  // Space
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_SPACE)) {
    m_inputText += ' ';
    return;
  }

  // Comma and period  // Check if Shift is pressed

  if (m_renderer->isKeyJustPressed(KeyCode::KEY_COMMA)) {
    m_inputText += shiftPressed ? '?' : ',';
    return;
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_PERIOD)) {
    m_inputText += shiftPressed ? '.' : ';';
    return;
  }

  // Slash and colon (on AZERTY, : is where / is on QWERTY, Shift+: gives /)
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_SLASH) || m_renderer->isKeyJustPressed(KeyCode::KEY_SEMICOLON)) {
    m_inputText += shiftPressed ? '/' : ':';
    return;
  }

  // Right parenthesis (usually close to 0)
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RIGHTPAREN)) {
    m_inputText += ')';
    return;
  }

  // Minus/underscore (already handled in number 6 and 8 for AZERTY)
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_MINUS)) {
    m_inputText += shiftPressed ? '_' : '-';
    return;
  }

  // Apostrophe/quote (already handled in number 4 for AZERTY)
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_APOSTROPHE)) {
    m_inputText += shiftPressed ? '"' : '\'';
    return;
  }
}

void ChatUI::update(float deltaTime)
{
  m_currentTime += deltaTime;
}

void ChatUI::render()
{
  if (!m_renderer || !m_font) {
    return;
  }

  int winHeight = m_renderer->getWindowHeight();

  if (m_isVisible) {
    // Full chat mode
    renderBackground();
    renderMessages();
    renderInputBox();
  } else {
    // Minimized mode: show recent messages with fade
    int yPos = winHeight - CHAT_MARGIN - INPUT_HEIGHT;
    int displayCount = 0;

    for (auto it = m_messages.rbegin(); it != m_messages.rend() && displayCount < 5; ++it, ++displayCount) {
      float age = m_currentTime - it->timestamp;
      if (age > MESSAGE_FADE_TIME) {
        break; // Stop showing old messages
      }

      // Calculate fade alpha
      float fadeStart = MESSAGE_FADE_TIME - 2.0F;
      uint8_t alpha = 255;
      if (age > fadeStart) {
        alpha = static_cast<uint8_t>(255.0F * (1.0F - (age - fadeStart) / 2.0F));
      }

      // Draw message with shadow for visibility
      std::string displayText = it->isSystem
        ? it->sender + ": " + it->content
        : "[" + std::to_string(it->senderId) + "] " + it->sender + ": " + it->content;
      Color shadowColor = {.r = 0, .g = 0, .b = 0, .a = alpha};
      Color textColor = it->isSystem ? Color{.r = 255, .g = 200, .b = 100, .a = alpha}
                                     : Color{.r = 255, .g = 255, .b = 255, .a = alpha};

      m_renderer->drawText(m_smallFont, displayText, CHAT_MARGIN + 1, yPos + 1, shadowColor);
      m_renderer->drawText(m_smallFont, displayText, CHAT_MARGIN, yPos, textColor);

      yPos -= MESSAGE_HEIGHT;
    }
  }
}

void ChatUI::renderBackground()
{
  int winHeight = m_renderer->getWindowHeight();
  int chatX = CHAT_MARGIN;
  int chatY = winHeight - CHAT_HEIGHT - CHAT_MARGIN;

  // Semi-transparent background
  Color bgColor = {.r = 20, .g = 20, .b = 30, .a = 200};
  m_renderer->drawRect(chatX, chatY, CHAT_WIDTH, CHAT_HEIGHT, bgColor);

  // Border
  Color borderColor = {.r = 80, .g = 80, .b = 100, .a = 255};
  m_renderer->drawRect(chatX, chatY, CHAT_WIDTH, 2, borderColor); // Top
  m_renderer->drawRect(chatX, chatY + CHAT_HEIGHT - 2, CHAT_WIDTH, 2, borderColor); // Bottom
  m_renderer->drawRect(chatX, chatY, 2, CHAT_HEIGHT, borderColor); // Left
  m_renderer->drawRect(chatX + CHAT_WIDTH - 2, chatY, 2, CHAT_HEIGHT, borderColor); // Right

  // Title bar
  Color titleBgColor = {.r = 40, .g = 40, .b = 60, .a = 255};
  m_renderer->drawRect(chatX, chatY, CHAT_WIDTH, 25, titleBgColor);

  Color titleColor = {.r = 200, .g = 200, .b = 220, .a = 255};
  m_renderer->drawText(m_font, "Chat (escape to close)", chatX + PADDING, chatY + 4, titleColor);
}

void ChatUI::renderMessages()
{
  int winHeight = m_renderer->getWindowHeight();
  int chatX = CHAT_MARGIN;
  int chatY = winHeight - CHAT_HEIGHT - CHAT_MARGIN;

  int messagesAreaY = chatY + 30;
  // int messagesAreaHeight = CHAT_HEIGHT - 30 - INPUT_HEIGHT - PADDING;

  // Render messages from bottom to top
  int yPos = chatY + CHAT_HEIGHT - INPUT_HEIGHT - PADDING - MESSAGE_HEIGHT;
  int displayCount = 0;

  for (auto it = m_messages.rbegin(); it != m_messages.rend() && displayCount < VISIBLE_MESSAGES;
       it++, displayCount++) {
    if (yPos < messagesAreaY) {
      break;
    }

    // Format: [ID] Sender: Content (or just Sender: Content for system messages)
    std::string displayText = it->isSystem
      ? it->sender + ": " + it->content
      : "[" + std::to_string(it->senderId) + "] " + it->sender + ": " + it->content;

    // Truncate if too long
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(m_smallFont, displayText, textWidth, textHeight);
    while (textWidth > CHAT_WIDTH - 2 * PADDING && !displayText.empty()) {
      displayText.pop_back();
      m_renderer->getTextSize(m_smallFont, displayText, textWidth, textHeight);
    }

    Color textColor =
      it->isSystem ? Color{.r = 255, .g = 200, .b = 100, .a = 255} : Color{.r = 220, .g = 220, .b = 230, .a = 255};

    m_renderer->drawText(m_smallFont, displayText, chatX + PADDING, yPos, textColor);
    yPos -= MESSAGE_HEIGHT;
  }
}

void ChatUI::renderInputBox()
{
  int winHeight = m_renderer->getWindowHeight();
  int chatX = CHAT_MARGIN;
  int chatY = winHeight - CHAT_HEIGHT - CHAT_MARGIN;

  int inputX = chatX + PADDING;
  int inputWidth = CHAT_WIDTH - 2 * PADDING;

  // Calculate number of lines based on text length (33 chars per line)
  static constexpr int CHARS_PER_LINE = 33;
  int textLength = static_cast<int>(m_inputText.size());
  int lineCount = std::max(1, (textLength + CHARS_PER_LINE - 1) / CHARS_PER_LINE);
  int dynamicInputHeight = INPUT_HEIGHT + (lineCount - 1) * 20; // 20 pixels per additional line

  int inputY = chatY + CHAT_HEIGHT - dynamicInputHeight - 5;

  // Input background
  Color inputBgColor =
    m_isInputFocused ? Color{.r = 50, .g = 50, .b = 70, .a = 255} : Color{.r = 30, .g = 30, .b = 45, .a = 255};
  m_renderer->drawRect(inputX - 2, inputY - 2, inputWidth + 4, dynamicInputHeight, inputBgColor);

  // Input border
  Color inputBorderColor =
    m_isInputFocused ? Color{.r = 100, .g = 150, .b = 255, .a = 255} : Color{.r = 60, .g = 60, .b = 80, .a = 255};
  m_renderer->drawRect(inputX - 2, inputY - 2, inputWidth + 4, 2, inputBorderColor);
  m_renderer->drawRect(inputX - 2, inputY + dynamicInputHeight - 4, inputWidth + 4, 2, inputBorderColor);
  m_renderer->drawRect(inputX - 2, inputY - 2, 2, dynamicInputHeight, inputBorderColor);
  m_renderer->drawRect(inputX + inputWidth, inputY - 2, 2, dynamicInputHeight, inputBorderColor);

  // Prepare display text with line breaks
  std::string displayText = m_inputText;
  if (m_isInputFocused) {
    // Blinking cursor
    int cursorPhase = static_cast<int>(m_currentTime * 2.0F) % 2;
    if (cursorPhase == 0) {
      displayText += "_";
    }
  }

  Color inputTextColor = {.r = 255, .g = 255, .b = 255, .a = 255};
  if (displayText.empty() && m_isInputFocused) {
    displayText = "Type a message...";
    inputTextColor = {.r = 128, .g = 128, .b = 140, .a = 255};
    m_renderer->drawText(m_font, displayText, inputX + 4, inputY + 4, inputTextColor);
  } else {
    // Render text with line wrapping
    int currentLine = 0;
    for (size_t i = 0; i < displayText.size(); i += CHARS_PER_LINE) {
      std::string line = displayText.substr(i, CHARS_PER_LINE);
      m_renderer->drawText(m_font, line, inputX + 4, inputY + 4 + currentLine * 20, inputTextColor);
      currentLine++;
    }
  }
}

void ChatUI::addMessage(const std::string &sender, const std::string &content, bool isSystem, std::uint32_t senderId)
{
  ChatMessage msg;
  msg.sender = sender;
  msg.content = content;
  msg.timestamp = m_currentTime;
  msg.isSystem = isSystem;
  msg.senderId = senderId;

  m_messages.push_back(msg);

  // Limit message history
  while (m_messages.size() > MAX_MESSAGES) {
    m_messages.pop_front();
  }

  std::cout << "[ChatUI] Message added from " << sender << " (ID: " << senderId << "): " << content << std::endl;
}

std::string ChatUI::consumeMessage()
{
  m_messageReady = false;
  std::string msg = m_inputText;
  m_inputText.clear();
  return msg;
}
