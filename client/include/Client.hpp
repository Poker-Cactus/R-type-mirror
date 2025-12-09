/*
 ** EPITECH PROJECT, 2025
 ** R-type-mirror
 ** File description:
 ** Client.hpp - Game client wrapper
 */

#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <atomic>
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

  /**
   * @brief Signal handler to stop the client
   *
   * @param signum Signal number
   */
  static void signalHandler(int signum);

private:
  std::shared_ptr<INetworkManager> m_networkManager;
  static std::atomic<bool> g_running;
};

#endif // CLIENT_HPP_
