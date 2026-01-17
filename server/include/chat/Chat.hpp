/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Chat.hpp - Server-side chat management with commands
*/

#pragma once

#include "../../../network/include/INetworkManager.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Server
{

/**
 * @brief Chat system with command handling
 * Manages mute/unmute and other chat commands
 */
class Chat
{
public:
  using CommandHandler = std::function<void(std::uint32_t senderId, const std::string &args)>;

  explicit Chat(std::shared_ptr<INetworkManager> networkManager);
  ~Chat() = default;

  /**
   * @brief Process a chat message and execute command if applicable
   * @param senderId Client ID of the sender
   * @param sender Username of the sender
   * @param content Message content
   * @return true if message was a command, false if regular chat
   */
  bool processMessage(std::uint32_t senderId, const std::string &sender, const std::string &content);

  /**
   * @brief Check if a user is muted
   * @param userId Client ID to check
   * @return true if user is muted
   */
  bool isMuted(std::uint32_t userId) const;

  /**
   * @brief Send a system message to a specific client
   * @param clientId Target client ID
   * @param message System message to send
   */
  void sendSystemMessage(std::uint32_t clientId, const std::string &message);

  /**
   * @brief Broadcast a system message to all clients
   * @param message System message to broadcast
   */
  void broadcastSystemMessage(const std::string &message);

  /**
   * @brief Set callback for when a player needs to be disconnected
   * @param callback Function to call when kicking a player
   */
  void setDisconnectCallback(std::function<void(std::uint32_t)> callback) { m_disconnectCallback = callback; }

private:
  void muteUserCommand(std::uint32_t senderId, const std::string &args);
  void unmuteUserCommand(std::uint32_t senderId, const std::string &args);
  void kickUserCommand(std::uint32_t senderId, const std::string &args);
  void helpCommand(std::uint32_t senderId, const std::string &args);

  std::unordered_map<std::string, CommandHandler> m_commandHandlers;
  std::unordered_set<std::uint32_t> m_mutedUsers; ///< Set of muted user IDs
  std::shared_ptr<INetworkManager> m_networkManager;
  std::function<void(std::uint32_t)> m_disconnectCallback; ///< Callback to disconnect/remove a player
};

} // namespace Server
