/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkReceiveSystem (Client)
*/

#ifndef CLIENT_NETWORKRECEIVESYSTEM_HPP_
#define CLIENT_NETWORKRECEIVESYSTEM_HPP_

#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <functional>
#include <nlohmann/json.hpp>

class ClientNetworkReceiveSystem : public ecs::ISystem
{
public:
  ClientNetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager);
  ~ClientNetworkReceiveSystem() override;

  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

  // Callbacks for game state
  void setGameStartedCallback(std::function<void()> callback);

  // Callbacks for lobby state
  void setLobbyJoinedCallback(std::function<void(const std::string &)> callback);
  void setLobbyStateCallback(std::function<void(const std::string &, int)> callback);
  void setErrorCallback(std::function<void(const std::string &)> callback);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  std::function<void()> m_gameStartedCallback;
  std::function<void(const std::string &)> m_lobbyJoinedCallback;
  std::function<void(const std::string &, int)> m_lobbyStateCallback;
  std::function<void(const std::string &)> m_errorCallback;

  void handleEntityCreated(ecs::World &world, const nlohmann::json &json);
  void handleEntityUpdate(ecs::World &world, const nlohmann::json &json);
  void handleSnapshot(ecs::World &world, const nlohmann::json &json);
  void handleGameStarted();
};

#endif /* !CLIENT_NETWORKRECEIVESYSTEM_HPP_ */
