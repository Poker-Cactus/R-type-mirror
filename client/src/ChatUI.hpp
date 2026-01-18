/**
 * @file ChatUI.hpp
 * @brief In-game chat interface.
 */

#ifndef CHATUI_HPP_
#define CHATUI_HPP_

#include "../interface/IRenderer.hpp"
#include <deque>
#include <memory>
#include <string>

/**
 * @struct ChatMessage
 * @brief Represents a single chat message
 */
struct ChatMessage {
  std::string sender; ///< Username of the sender
  std::string content; ///< Message content
  float timestamp; ///< Time when message was received
  bool isSystem; ///< True if it's a system message
  std::uint32_t senderId; ///< ID of the sender (0 for system messages)
};

/**
 * @class ChatUI
 * @brief In-game chat interface that can be toggled with Ctrl+T
 *
 * Provides a chat overlay that allows players to communicate
 * with all other players connected to the same server.
 */
class ChatUI
{
public:
  /**
   * @brief Construct ChatUI with renderer reference
   * @param renderer Shared pointer to the renderer
   */
  explicit ChatUI(std::shared_ptr<IRenderer> renderer);
  ~ChatUI() = default;

  /**
   * @brief Initialize chat resources (fonts, etc.)
   * @return true if initialization succeeded
   */
  bool init();

  /**
   * @brief Toggle chat visibility
   */
  void toggle();

  /**
   * @brief Open the chat
   */
  void open();

  /**
   * @brief Close the chat
   */
  void close();

  /**
   * @brief Check if chat is currently visible
   * @return true if chat is open
   */
  [[nodiscard]] bool isVisible() const { return m_isVisible; }

  /**
   * @brief Check if chat input is focused (capturing text input)
   * @return true if input is focused
   */
  [[nodiscard]] bool isInputFocused() const { return m_isInputFocused; }

  /**
   * @brief Process input events for the chat
   */
  void processInput();

  /**
   * @brief Update chat state
   * @param deltaTime Time since last frame
   */
  void update(float deltaTime);

  /**
   * @brief Render the chat interface
   */
  void render();

  /**
   * @brief Add a message to the chat
   * @param sender Username of the sender
   * @param content Message content
   * @param isSystem True if it's a system message
   * @param senderId ID of the sender (0 for system messages)
   */
  void addMessage(const std::string &sender, const std::string &content, bool isSystem = false,
                  std::uint32_t senderId = 0);

  /**
   * @brief Get the current input text
   * @return Current input string
   */
  [[nodiscard]] const std::string &getInputText() const { return m_inputText; }

  /**
   * @brief Clear the input text after sending
   */
  void clearInput() { m_inputText.clear(); }

  /**
   * @brief Check if a message is ready to be sent
   * @return true if Enter was pressed with non-empty input
   */
  [[nodiscard]] bool hasMessageToSend() const { return m_messageReady; }

  /**
   * @brief Get and consume the pending message
   * @return The message to send
   */
  std::string consumeMessage();

  /**
   * @brief Set the local player's username
   * @param username Player's username
   */
  void setLocalUsername(const std::string &username) { m_localUsername = username; }

private:
  /** @brief Render the chat background panel. */
  void renderBackground();
  /** @brief Render the chat message list. */
  void renderMessages();
  /** @brief Render the chat input box. */
  void renderInputBox();
  /** @brief Handle text input and key mapping. */
  void handleTextInput();

  std::shared_ptr<IRenderer> m_renderer;
  void *m_font = nullptr;
  void *m_smallFont = nullptr;

  bool m_isVisible = false;
  bool m_isInputFocused = false;
  bool m_messageReady = false;

  std::string m_inputText;
  std::string m_localUsername = "Player";
  std::deque<ChatMessage> m_messages;
  float m_currentTime = 0.0F;

  // UI Constants
  static constexpr int MAX_MESSAGES = 50;
  static constexpr int VISIBLE_MESSAGES = 10;
  static constexpr int CHAT_WIDTH = 400;
  static constexpr int CHAT_HEIGHT = 300;
  static constexpr int CHAT_MARGIN = 20;
  static constexpr int MESSAGE_HEIGHT = 22;
  static constexpr int INPUT_HEIGHT = 30;
  static constexpr int PADDING = 10;
  static constexpr size_t MAX_INPUT_LENGTH = 200;
  static constexpr float MESSAGE_FADE_TIME = 10.0F; ///< Messages fade after this time when chat is closed
};

#endif // CHATUI_HPP_
