/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Client.hpp - Game client wrapper
*/

#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <iostream>
#include <memory>
#include <thread>

#include "../../common/include/Common.hpp"
#include "../../network/include/INetworkManager.hpp"

/**
 * @brief High-level client wrapper
 *
 * Provides main game loop for client-side networking.
 */
class Client
{
public:
  /**
   * @brief Construct a client
   *
   * @param networkManager The network manager to use
   */
  explicit Client(std::shared_ptr<INetworkManager> networkManager);
  ~Client();

  /**
   * @brief Run the client main loop
   */
  void loop();

private:
  std::shared_ptr<INetworkManager> m_networkManager;
};

#endif // CLIENT_HPP_
