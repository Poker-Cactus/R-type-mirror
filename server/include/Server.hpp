/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Server.hpp - Game server wrapper
*/

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <atomic>
#include <memory>

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

  /**
   * @brief Run the server main loop
   */
  void loop();

  /**
   * @brief Signal handler to stop the server
   *
   * @param signum Signal number
   */
  static void signalHandler(int signum);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  static std::atomic<bool> g_running;
};

#endif // SERVER_HPP_
