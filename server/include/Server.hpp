/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Server.hpp - Game server wrapper
*/

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <memory>

#include "../../common/include/Common.hpp"
#include "../../network/include/INetworkManager.hpp"

/**
 * @brief High-level server wrapper
 *
 * Provides main game loop for server-side networking.
 */
class Server
{
public:
  /**
   * @brief Construct a server
   *
   * @param networkManager The network manager to use
   */
  explicit Server(std::shared_ptr<INetworkManager> networkManager);
  ~Server();

private:
  std::shared_ptr<INetworkManager> m_networkManager;
};

#endif // SERVER_HPP_
