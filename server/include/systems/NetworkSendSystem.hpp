/**
 * @file NetworkSendSystem.hpp
 * @brief Server-side network send system.
 */

#ifndef NETWORKSENDSYSTEM_HPP_
#define NETWORKSENDSYSTEM_HPP_
#include "../../engineCore/include/ecs/ISystem.hpp"
#include "../../network/include/INetworkManager.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class LobbyManager;

/**
 * @class NetworkSendSystem
 * @brief Server system that broadcasts world state to clients.
 */
class NetworkSendSystem : public ecs::ISystem
{
public:
  /** @brief Construct the send system. */
  NetworkSendSystem(std::shared_ptr<INetworkManager> networkManager);
  /** @brief Destroy the send system. */
  ~NetworkSendSystem();
  /** @brief Send snapshots and events for this frame. */
  void update(ecs::World &world, float deltaTime) override;
  /** @brief Get the component signature for this system. */
  [[nodiscard]] ecs::ComponentSignature getSignature() const override;

  /**
   * @brief Set the lobby manager for filtering active game clients
   * @param lobbyManager Pointer to the lobby manager
   */
  void setLobbyManager(LobbyManager *lobbyManager);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  LobbyManager *m_lobbyManager = nullptr;
  float m_timeSinceLastSend = 0.0f;

  // Track network IDs per lobby for destroyed entity detection
  std::unordered_map<std::string, std::vector<uint32_t>> m_lobbyLastNetworkIds;

  static constexpr float SEND_INTERVAL = 0.016f;

  /**
   * @brief Get all clients that are in an active game
   * @return Vector of client IDs in active games
   */
  [[nodiscard]] std::vector<std::uint32_t> getActiveGameClients() const;
};

#endif /* !NETWORKSENDSYSTEM_HPP_ */
