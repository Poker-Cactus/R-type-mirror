/**
 * @file LobbyManager.cpp
 * @brief Lobby manager implementation.
 */

#include "LobbyManager.hpp"
#include <iostream>

bool LobbyManager::createLobby(const std::string &code, GameConfig::Difficulty difficulty, bool isSolo,
                               AIDifficulty aiDifficulty, GameMode mode)
{
  if (m_lobbies.find(code) != m_lobbies.end()) {
    return false;
  }

  std::cout << "[LobbyManager] Creating " << (isSolo ? "SOLO " : "") << "lobby '" << code << "' with difficulty "
            << static_cast<int>(difficulty) << " and AI difficulty " << static_cast<int>(aiDifficulty)
            << " and game mode " << (mode == GameMode::CLASSIC ? "CLASSIC" : "ENDLESS") << '\n';

  // Pass the network manager (if any) to the lobby so it can send direct messages
  m_lobbies[code] = std::make_unique<Lobby>(code, m_networkManager, isSolo, aiDifficulty, mode);

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
  m_lobbies[code]->setGameMode(mode);

  // Let the lobby know its manager for callbacks
  m_lobbies[code]->setManager(this);

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

void LobbyManager::removeLobby(const std::string &code)
{
  auto it = m_lobbies.find(code);
  if (it == m_lobbies.end()) {
    return;
  }

  std::cout << "[LobbyManager] Removing lobby by request: " << code << '\n';

  // Notify all clients in the lobby that the lobby is closing
  Lobby *lobby = it->second.get();
  if (lobby) {
    std::vector<std::uint32_t> clients(lobby->getClients().begin(), lobby->getClients().end());
    nlohmann::json notice;
    notice["type"] = "lobby_closed";
    notice["message"] = "Lobby closed: all players dead";
    for (auto cid : clients) {
      lobby->sendJsonToClient(cid, notice);
      // Remove client->lobby mapping
      leaveLobby(cid);
    }

    // Stop the game if running
    if (lobby->isGameStarted()) {
      lobby->stopGame();
    }
  }

  // Erase the lobby
  m_lobbies.erase(it);
}

const std::unordered_map<std::string, std::unique_ptr<Lobby>> &LobbyManager::getLobbies() const
{
  return m_lobbies;
}
