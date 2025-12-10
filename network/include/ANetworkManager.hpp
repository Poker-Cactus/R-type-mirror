/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ANetworkManager.hpp - Abstract network manager base class
*/

#ifndef A_NETWORK_MANAGER_HPP_
#define A_NETWORK_MANAGER_HPP_

#include <memory>

#include "INetworkManager.hpp"
#include "IPacketHandler.hpp"

/**
 * @brief Abstract base class for network managers
 *
 * Provides common functionality for network manager implementations.
 */
class ANetworkManager : public INetworkManager
{
public:
  /**
   * @brief Construct with a packet handler
   *
   * @param packetHandler The packet handler to use
   */
  explicit ANetworkManager(std::shared_ptr<IPacketHandler> packetHandler);
  virtual ~ANetworkManager() = default;

  /**
   * @brief Get the packet handler
   *
   * @return Shared pointer to the packet handler
   */
  [[nodiscard]] std::shared_ptr<IPacketHandler> getPacketHandler() const override { return m_packetHandler; }

protected:
  std::shared_ptr<IPacketHandler> m_packetHandler;
};

#endif // A_NETWORK_MANAGER_HPP_
