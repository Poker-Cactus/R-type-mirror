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
#include <nlohmann/json.hpp>

class ClientNetworkReceiveSystem : public ecs::ISystem
{
public:
  ClientNetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager);
  ~ClientNetworkReceiveSystem() override;

  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

private:
  std::shared_ptr<INetworkManager> m_networkManager;

  void handleEntityCreated(ecs::World &world, const nlohmann::json &json);
  void handleEntityUpdate(ecs::World &world, const nlohmann::json &json);
  void handleSnapshot(ecs::World &world, const nlohmann::json &json);
};

#endif /* !CLIENT_NETWORKRECEIVESYSTEM_HPP_ */
