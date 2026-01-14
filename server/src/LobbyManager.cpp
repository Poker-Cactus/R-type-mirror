/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** LobbyManager implementation
*/

#include "LobbyManager.hpp"
#include <iostream>

bool LobbyManager::createLobby(const std::string &code, GameConfig::Difficulty difficulty, bool isSolo)
{
  if (m_lobbies.find(code) != m_lobbies.end()) {
    return false;
  }

  std::cout << "[LobbyManager] Creating " << (isSolo ? "SOLO " : "") << "lobby '" << code << "' with difficulty " << static_cast<int>(difficulty)
            << '\n';

  // Pass the network manager (if any) to the lobby so it can send direct messages
  m_lobbies[code] = std::make_unique<Lobby>(code, m_networkManager, isSolo);

  // Pass enemy config manager to the lobby
  if (m_enemyConfigManager) {
    m_lobbies[code]->setEnemyConfigManager(m_enemyConfigManager);
  }

  // Pass level config manager to the lobby
  if (m_levelConfigManager) {
    m_lobbies[code]->setLevelConfigManager(m_levelConfigManager);
  }

  // Set the difficulty before the game starts
  m_lobbies[code]->setDifficulty(difficulty);

  std::cout << "[LobbyManager] Created lobby: " << code << '\n';
  return true;
}

bool LobbyManager::joinLobby(const std::string &code, std::uint32_t clientId, bool asSpectator)
{
  auto lobby_it = m_lobbies.find(code);
  if (lobby_it == m_lobbies.end()) {
    std::cerr << "[LobbyManager] Lobby not found: " << code << '\n';
    return false;
  }

  leaveLobby(clientId);

  if (lobby_it->second->addClient(clientId, asSpectator)) {
    m_clientToLobby[clientId] = code;
    return true;
  }

  return false;
}

void LobbyManager::leaveLobby(std::uint32_t clientId)
{
  auto lobby_map_it = m_clientToLobby.find(clientId);
  if (lobby_map_it == m_clientToLobby.end()) {
    return;
  }

  // Just remove from tracking - actual lobby removal is handled by caller
  m_clientToLobby.erase(lobby_map_it);
}

Lobby *LobbyManager::getClientLobby(std::uint32_t clientId)
{
  auto lobby_map_it = m_clientToLobby.find(clientId);
  if (lobby_map_it == m_clientToLobby.end()) {
    return nullptr;
  }

  auto lobby_it = m_lobbies.find(lobby_map_it->second);
  return lobby_it != m_lobbies.end() ? lobby_it->second.get() : nullptr;
}

Lobby *LobbyManager::getLobby(const std::string &code)
{
  auto lobby_it = m_lobbies.find(code);
  return lobby_it != m_lobbies.end() ? lobby_it->second.get() : nullptr;
}

void LobbyManager::cleanupEmptyLobbies()
{
  for (auto lobby_it = m_lobbies.begin(); lobby_it != m_lobbies.end();) {
    if (lobby_it->second && lobby_it->second->isEmpty()) {
      std::cout << "[LobbyManager] Removing empty lobby: " << lobby_it->first << '\n';

      // Explicitly stop game before destruction to ensure clean shutdown
      if (lobby_it->second->isGameStarted()) {
        lobby_it->second->stopGame();
      }

      // Erase will trigger the unique_ptr destructor which calls ~Lobby()
      lobby_it = m_lobbies.erase(lobby_it);
    } else {
      ++lobby_it;
    }
  }
}

const std::unordered_map<std::string, std::unique_ptr<Lobby>> &LobbyManager::getLobbies() const
{
  return m_lobbies;
}
