/**
 * @file NetworkSendSystem.hpp
 * @brief Client-side input transmission system
 */

#ifndef NETWORKSENDSYSTEM_HPP_
#define NETWORKSENDSYSTEM_HPP_
#include "../../common/include/Common.hpp"
#include "../../engineCore/include/ecs/Entity.hpp"
#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <nlohmann/json.hpp>

/**
 * @class NetworkSendSystem
 * @brief Client-side ECS system for transmitting player inputs to server
 *
 * This system reads local Input components and sends them to the server.
 * It does NOT execute gameplay logic - only input transmission.
 *
 * Protocol: Sends JSON messages with type "player_input" containing:
 * - entity_id: The player entity ID
 * - input: Object with up, down, left, right, shoot booleans
 */
class NetworkSendSystem : public ecs::ISystem
{
public:
  /**
   * @brief Construct the network send system
   * @param networkManager Shared pointer to network manager
   */
  NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager);
  ~NetworkSendSystem();

  /**
   * @brief Update system - sends player inputs to server
   * @param world ECS world reference
   * @param deltaTime Time elapsed since last update
   */
  void update(ecs::World &world, float deltaTime) override;

  /**
   * @brief Get component signature for this system
   * @return Component signature
   */
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

  /**
   * @brief Set the client ID for this player
   * @param clientId Unique client identifier
   */
  void setClientId(std::uint32_t clientId) { m_clientId = clientId; }

  /**
   * @brief Get the current client ID
   * @return Client identifier
   */
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
