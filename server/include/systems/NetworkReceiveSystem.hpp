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

class NetworkReceiveSystem : public ecs::ISystem
{
public:
  NetworkReceiveSystem(std::shared_ptr<INetworkManager> networkManager);
  ~NetworkReceiveSystem();
  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

protected:
private:
  std::shared_ptr<INetworkManager> m_networkManager;
  void handlePlayerInput(ecs::World &world, std::string message, std::uint32_t clientId);
};

#endif /* !NETWORKReceiveSYSTEM_HPP_ */
