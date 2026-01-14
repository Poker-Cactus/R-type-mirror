/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem - FlappyBird Server
*/

#pragma once

#include "../../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../../network/include/INetworkManager.hpp"
#include <nlohmann/json.hpp>

class Game;

class NetworkReceiveSystem : public ecs::ISystem
{
public:
  NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager, Game *game);
  ~NetworkReceiveSystem();

  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  Game *m_game = nullptr;

  // Message handling
  void handleMessage(ecs::World &world, const std::string &message, std::uint32_t clientId);
  void handlePlayerInput(ecs::World &world, const std::string &message, std::uint32_t clientId);
  void handleConnect(std::uint32_t clientId);
  void handleDisconnect(std::uint32_t clientId);

  // Network helpers
  void sendJsonMessage(std::uint32_t clientId, const nlohmann::json &message);
};
