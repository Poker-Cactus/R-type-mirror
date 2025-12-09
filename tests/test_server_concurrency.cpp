/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** test_server_concurrency.cpp
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "AsioClient.hpp"
#include "AsioServer.hpp"
#include <atomic>
#include <chrono>
#include <doctest/doctest.h>
#include <iostream>
#include <thread>
#include <vector>

TEST_CASE("Server Concurrency and Async Handling")
{
  // Use a port likely to be free
  constexpr short TEST_SERVER_PORT = 5000;
  short port = TEST_SERVER_PORT;

  std::cout << "Starting Server on port " << port << "..." << '\n';

  // 1. Start Server
  std::shared_ptr<AsioServer> server;
  try {
    server = std::make_shared<AsioServer>(port);
    server->start();
  } catch (const std::exception &e) {
    FAIL("Could not start server: " << e.what());
  }

  // Atomic counter for received messages
  std::atomic<int> received_count(0);

  // 2. Prepare Clients
  int num_threads = 4;
  constexpr int MSGS_PER_THREAD = 50;
  int msgs_per_thread = MSGS_PER_THREAD;
  int total_expected = num_threads * msgs_per_thread;

  std::vector<std::thread> client_threads;

  // Barrier to start all threads roughly at same time
  std::atomic<bool> start_flag(false);

  std::cout << "Spawning " << num_threads << " threads sending " << msgs_per_thread << " messages each..." << '\n';

  client_threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    client_threads.emplace_back([port, msgs_per_thread, &start_flag]() {
      try {
        // Create client
        auto client = std::make_shared<AsioClient>("127.0.0.1", std::to_string(port));
        client->start();

        // Wait for signal
        while (!start_flag) {
          std::this_thread::yield();
        }

        constexpr int MICROSECONDS_BETWEEN_SENDS = 500;
        // Send burst
        for (int j = 0; j < msgs_per_thread; ++j) {
          auto serialized = client->getPacketHandler()->serialize("PING");
          client->send(
            std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

          // Small sleep to avoid overwhelming local UDP buffer too fast
          std::this_thread::sleep_for(std::chrono::microseconds(MICROSECONDS_BETWEEN_SENDS));
        }

        // Keep client alive for a bit to ensure sends go out
        constexpr int CLIENT_KEEP_ALIVE_MS = 500;
        std::this_thread::sleep_for(std::chrono::milliseconds(CLIENT_KEEP_ALIVE_MS));
        client->stop();
      } catch (const std::exception &e) {
        std::cerr << "Client thread error: " << e.what() << '\n';
      }
    });
  }

  // 3. Start the load
  start_flag = true;

  // 4. Poll Server
  // We give it some time to process everything
  constexpr int SERVER_POLL_TIMEOUT_SECONDS = 5;
  auto start_time = std::chrono::steady_clock::now();
  constexpr int SERVER_POLL_SLEEP_MS = 10;
  while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(SERVER_POLL_TIMEOUT_SECONDS)) {
    NetworkPacket msg;
    while (server->poll(msg)) {
      received_count++;
    }
    if (received_count >= total_expected) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_POLL_SLEEP_MS));
  }

  std::cout << "Received " << received_count << " / " << total_expected << " messages." << '\n';

  // 5. Cleanup
  for (auto &thr : client_threads) {
    if (thr.joinable()) {
      thr.join();
    }
  }
  server->stop();

  // 6. Assertions
  // We allow some packet loss in UDP, but locally it should be minimal.
  // Let's require at least 90% success to be robust against minor OS buffer drops.
  constexpr double MIN_SUCCESS_RATE = 0.9;
  CHECK(received_count >= total_expected * MIN_SUCCESS_RATE);
  CHECK(received_count <= total_expected); // Should not receive more
}
