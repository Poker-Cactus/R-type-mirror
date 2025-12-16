/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem
*/

#ifndef NETWORKReceiveSYSTEM_HPP_
#define NETWORKReceiveSYSTEM_HPP_
#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <nlohmann/json.hpp>

class Game;

class NetworkReceiveSystem : public ecs::ISystem
{
public:
  NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager);
  ~NetworkReceiveSystem();
  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;
  void setGame(Game *game);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  Game *m_game = nullptr;

  // Message handling
  void handleMessage(ecs::World &world, const std::string &message, std::uint32_t clientId);
  void handlePlayerInput(ecs::World &world, std::string message, std::uint32_t clientId);
  void handleViewport(ecs::World &world, const nlohmann::json &json, std::uint32_t clientId);
  void handleRequestLobby(const nlohmann::json &json, std::uint32_t clientId);
  void handleStartGame(ecs::World &world, std::uint32_t clientId);
  void handleLeaveLobby(ecs::World &world, std::uint32_t clientId);

  // Network helpers - properly serialize with Cap'n Proto
  void sendJsonMessage(std::uint32_t clientId, const nlohmann::json &message);
  void sendJsonMessageToAll(const std::vector<std::uint32_t> &clientIds, const nlohmann::json &message);
  void sendLobbyResponse(std::uint32_t clientId, const std::string &type, const std::string &code);
  void sendErrorResponse(std::uint32_t clientId, const std::string &error);
};

#endif /* !NETWORKReceiveSYSTEM_HPP_ */
