/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem
*/

#ifndef NETWORKSENDSYSTEM_HPP_
#define NETWORKSENDSYSTEM_HPP_
#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <vector>
#include <cstdint>

class NetworkSendSystem : public ecs::ISystem
{
public:
  NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager);
  ~NetworkSendSystem();
  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

protected:
private:
  std::shared_ptr<INetworkManager> m_networkManager;
  float m_timeSinceLastSend = 0.0f;
  std::vector<uint32_t> m_lastNetworkIds;
  static constexpr float SEND_INTERVAL = 0.016f;
};

#endif /* !NETWORKSENDSYSTEM_HPP_ */
