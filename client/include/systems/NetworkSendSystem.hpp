/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkSendSystem - Sends player inputs to server
*/

#ifndef NETWORKSENDSYSTEM_HPP_
#define NETWORKSENDSYSTEM_HPP_
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "../../common/include/Common.hpp"
#include <nlohmann/json.hpp>

/**
 * @brief Client-side system that sends player inputs to the server
 *
 * This system reads local Input components and transmits them to the server.
 * It does NOT execute any gameplay logic - only input transmission.
 *
 * Protocol: Sends JSON messages with type "player_input" containing:
 * - entity_id: The player entity ID
 * - input: Object with up, down, left, right, shoot booleans
 */
class NetworkSendSystem : public ecs::ISystem
{
public:
  NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager);
  ~NetworkSendSystem();
  void update(ecs::World &world, float deltaTime) override;
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

  void setClientId(std::uint32_t clientId) { m_clientId = clientId; }
  std::uint32_t getClientId() const { return m_clientId; }
  void sendSetDifficulty(Difficulty diff);

protected:
private:
  std::shared_ptr<INetworkManager> m_networkManager;
  std::uint32_t m_clientId = 0;

  /**
   * @brief Send input data to server
   *
   * @param entity The entity with the input
   * @param input The input component data
   */
  void sendInputToServer(ecs::Entity entity, const ecs::Input &input);
};

#endif /* !NETWORKSENDSYSTEM_HPP_ */
