/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LobbyManager implementation
*/

#include "LobbyManager.hpp"
#include <iostream>

bool LobbyManager::createLobby(const std::string &code)
{
  if (m_lobbies.find(code) != m_lobbies.end()) {
    return false;
  }

  m_lobbies[code] = std::make_unique<Lobby>(code);
  std::cout << "[LobbyManager] Created lobby: " << code << std::endl;
  return true;
}

bool LobbyManager::joinLobby(const std::string &code, std::uint32_t clientId)
{
  auto it = m_lobbies.find(code);
  if (it == m_lobbies.end()) {
    std::cerr << "[LobbyManager] Lobby not found: " << code << std::endl;
    return false;
  }

  leaveLobby(clientId);

  if (it->second->addClient(clientId)) {
    m_clientToLobby[clientId] = code;
    return true;
  }

  return false;
}

void LobbyManager::leaveLobby(std::uint32_t clientId)
{
  auto it = m_clientToLobby.find(clientId);
  if (it == m_clientToLobby.end()) {
    return;
  }

  // Just remove from tracking - actual lobby removal is handled by caller
  m_clientToLobby.erase(it);
}

Lobby *LobbyManager::getClientLobby(std::uint32_t clientId)
{
  auto it = m_clientToLobby.find(clientId);
  if (it == m_clientToLobby.end()) {
    return nullptr;
  }

  auto lobbyIt = m_lobbies.find(it->second);
  return lobbyIt != m_lobbies.end() ? lobbyIt->second.get() : nullptr;
}

Lobby *LobbyManager::getLobby(const std::string &code)
{
  auto it = m_lobbies.find(code);
  return it != m_lobbies.end() ? it->second.get() : nullptr;
}

void LobbyManager::cleanupEmptyLobbies()
{
  for (auto it = m_lobbies.begin(); it != m_lobbies.end();) {
    if (it->second && it->second->isEmpty()) {
      std::cout << "[LobbyManager] Removing empty lobby: " << it->first << std::endl;

      // Explicitly stop game before destruction to ensure clean shutdown
      if (it->second->isGameStarted()) {
        it->second->stopGame();
      }

      // Erase will trigger the unique_ptr destructor which calls ~Lobby()
      it = m_lobbies.erase(it);
    } else {
      ++it;
    }
  }
}

const std::unordered_map<std::string, std::unique_ptr<Lobby>> &LobbyManager::getLobbies() const
{
  return m_lobbies;
}
