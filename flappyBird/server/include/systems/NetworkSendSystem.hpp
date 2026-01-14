/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem - FlappyBird Server
*/

#pragma once

#include "../../../../engineCore/include/ecs/ISystem.hpp"
#include "../../../../network/include/INetworkManager.hpp"

class NetworkSendSystem : public ecs::ISystem
{
public:
  explicit NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager);
  ~NetworkSendSystem();

  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

private:
  std::shared_ptr<INetworkManager> m_networkManager;
};
