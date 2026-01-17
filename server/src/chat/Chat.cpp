/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Chat.cpp - Server-side chat implementation
*/

#include "../../include/chat/Chat.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace Server
{

Chat::Chat(std::shared_ptr<INetworkManager> networkManager) : m_networkManager(std::move(networkManager))
{
  // Initialize command handlers using a map of string -> lambda
  m_commandHandlers = {
    {"mute", [this](std::uint32_t senderId, const std::string &args) { muteUserCommand(senderId, args); }},
    {"unmute", [this](std::uint32_t senderId, const std::string &args) { unmuteUserCommand(senderId, args); }},
    {"kick", [this](std::uint32_t senderId, const std::string &args) { kickUserCommand(senderId, args); }},
    {"help", [this](std::uint32_t senderId, const std::string &args) { helpCommand(senderId, args); }},
  };
}

bool Chat::processMessage(std::uint32_t senderId, const std::string & /*sender*/, const std::string &content)
{
  // Check if message starts with '-'
  if (content.empty() || content[0] != '-') {
    return false; // Not a command
  }

  // Parse command and arguments
  std::istringstream iss(content.substr(1)); // Skip the '-'
  std::string command;
  iss >> command;

  // Get remaining arguments
  std::string args;
  std::getline(iss >> std::ws, args);

  // Find and execute command handler
  auto it = m_commandHandlers.find(command);
  if (it != m_commandHandlers.end()) {
    it->second(senderId, args);
    return true; // Command processed
  }

  // Unknown command
  sendSystemMessage(senderId, "Unknown command: -" + command + ". Type -help for available commands.");
  return true;
}

bool Chat::isMuted(std::uint32_t userId) const
{
  return m_mutedUsers.find(userId) != m_mutedUsers.end();
}

void Chat::sendSystemMessage(std::uint32_t clientId, const std::string &message)
{
  if (!m_networkManager) {
    return;
  }

  nlohmann::json systemMsg;
  systemMsg["type"] = "chat_broadcast";
  systemMsg["sender"] = "System";
  systemMsg["content"] = message;
  systemMsg["senderId"] = 0; // System ID

  std::string jsonStr = systemMsg.dump();
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), clientId);
}

void Chat::broadcastSystemMessage(const std::string &message)
{
  if (!m_networkManager) {
    return;
  }

  nlohmann::json systemMsg;
  systemMsg["type"] = "chat_broadcast";
  systemMsg["sender"] = "System";
  systemMsg["content"] = message;
  systemMsg["senderId"] = 0; // System ID

  std::string jsonStr = systemMsg.dump();
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);

  // Broadcast to all clients (clientId = 0 means broadcast in most implementations)
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);
}

void Chat::muteUserCommand(std::uint32_t senderId, const std::string &args)
{
  if (args.empty()) {
    sendSystemMessage(senderId, "Usage: -mute <player_id>");
    return;
  }

  // For simplicity, using client ID
  std::uint32_t targetClientId = 0;
  try {
    targetClientId = static_cast<std::uint32_t>(std::stoul(args));
  } catch (...) {
    sendSystemMessage(senderId, "Invalid player ID. Usage: -mute <player_id>");
    return;
  }

  if (targetClientId == senderId) {
    sendSystemMessage(senderId, "You cannot mute yourself!");
    return;
  }

  if (targetClientId == 0) {
    sendSystemMessage(senderId, "Cannot mute system!");
    return;
  }

  // Mute the user
  m_mutedUsers.insert(targetClientId);
  sendSystemMessage(senderId, "Player " + args + " has been muted.");
  sendSystemMessage(targetClientId, "You have been muted by an admin.");

  std::cout << "[Chat] Client " << senderId << " muted client " << targetClientId << std::endl;
}

void Chat::unmuteUserCommand(std::uint32_t senderId, const std::string &args)
{
  if (args.empty()) {
    sendSystemMessage(senderId, "Usage: -unmute <player_id>");
    return;
  }

  // For simplicity, using client ID
  std::uint32_t targetClientId = 0;
  try {
    targetClientId = static_cast<std::uint32_t>(std::stoul(args));
  } catch (...) {
    sendSystemMessage(senderId, "Invalid player ID. Usage: -unmute <player_id>");
    return;
  }

  if (m_mutedUsers.find(targetClientId) == m_mutedUsers.end()) {
    sendSystemMessage(senderId, "Player " + args + " is not muted.");
    return;
  }

  // Unmute the user
  m_mutedUsers.erase(targetClientId);
  sendSystemMessage(senderId, "Player " + args + " has been unmuted.");
  sendSystemMessage(targetClientId, "You have been unmuted.");

  std::cout << "[Chat] Client " << senderId << " unmuted client " << targetClientId << std::endl;
}

void Chat::helpCommand(std::uint32_t senderId, const std::string & /*args*/)
{
  std::string helpText = "Available commands:\n";
  helpText += "-mute <player_id> - Mute a player\n";
  helpText += "-unmute <player_id> - Unmute a player\n";
  helpText += "-kick <player_id> - Kick a player from the game\n";
  helpText += "-help - Show this help message";

  sendSystemMessage(senderId, helpText);
}

void Chat::kickUserCommand(std::uint32_t senderId, const std::string &args)
{
  if (args.empty()) {
    sendSystemMessage(senderId, "Usage: -kick <player_id>");
    return;
  }

  // Parse player ID
  std::uint32_t targetClientId = 0;
  try {
    targetClientId = static_cast<std::uint32_t>(std::stoul(args));
  } catch (...) {
    sendSystemMessage(senderId, "Invalid player ID. Usage: -kick <player_id>");
    return;
  }

  if (targetClientId == senderId) {
    sendSystemMessage(senderId, "You cannot kick yourself!");
    return;
  }

  if (targetClientId == 0) {
    sendSystemMessage(senderId, "Cannot kick system!");
    return;
  }

  // Check if target client exists
  auto clients = m_networkManager->getClients();
  if (clients.find(targetClientId) == clients.end()) {
    sendSystemMessage(senderId, "Player " + args + " not found.");
    return;
  }

  // Notify the kicked player before disconnecting
  nlohmann::json kickMsg;
  kickMsg["type"] = "player_kicked";
  kickMsg["reason"] = "You have been kicked from the game.";
  std::string kickJsonStr = kickMsg.dump();
  auto kickSerialized = m_networkManager->getPacketHandler()->serialize(kickJsonStr);
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(kickSerialized.data()), kickSerialized.size()),
    targetClientId);

  sendSystemMessage(targetClientId, "You have been kicked from the game.");

  // Use callback to properly disconnect and cleanup (removes from lobby and destroys entity)
  if (m_disconnectCallback) {
    m_disconnectCallback(targetClientId);
  } else {
    // Fallback: just disconnect from network
    m_networkManager->disconnect(targetClientId);
  }

  // Notify the sender and broadcast
  sendSystemMessage(senderId, "Player " + args + " has been kicked.");
  broadcastSystemMessage("Player " + args + " was kicked from the game.");

  std::cout << "[Chat] Client " << senderId << " kicked client " << targetClientId << std::endl;
}

} // namespace Server
