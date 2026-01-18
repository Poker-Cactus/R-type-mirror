/**
 * @file Client.hpp
 * @brief High-level game client wrapper.
 */

#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <atomic>
#include <iostream>
#include <memory>
#include <thread>

#include "../../common/include/Common.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "Settings.hpp"

/**
 * @brief High-level client wrapper
 *
 * Provides main game loop for client-side networking with ECS architecture.
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
  /**
   * @brief Register and configure ECS/network systems.
   */
  void initializeSystems();

  std::shared_ptr<INetworkManager> m_networkManager;
  std::shared_ptr<ecs::World> m_world;
  static std::atomic<bool> g_running;
};

#endif // CLIENT_HPP_
