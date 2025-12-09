/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Test_server_stress.cpp
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

TEST_CASE("Server Stress Test")
{
  // Use a different port for stress test
  short port = 5001;

  std::cout << "Starting Stress Test Server on port " << port << "..." << std::endl;

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
  // Increase load: 20 threads, 500 messages each = 10,000 messages
  int num_threads = 20;
  int msgs_per_thread = 500;
  int total_expected = num_threads * msgs_per_thread;

  std::vector<std::thread> client_threads;

  // Barrier to start all threads roughly at same time
  std::atomic<bool> start_flag(false);

  std::cout << "Spawning " << num_threads << " threads sending " << msgs_per_thread << " messages each..." << std::endl;
  std::cout << "Total expected messages: " << total_expected << std::endl;

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

        // Send burst
        for (int j = 0; j < msgs_per_thread; ++j) {
          auto serialized = client->getPacketHandler()->serialize("STRESS");
          client->send(
            std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), 0);

          // Minimal sleep to allow some interleaving but keep pressure high
          // 100 microseconds
          std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        // Keep client alive for a bit to ensure sends go out
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        client->stop();
      } catch (const std::exception &e) {
        std::cerr << "Client thread error: " << e.what() << std::endl;
      }
    });
  }

  // 3. Start the load
  start_flag = true;

  // 4. Poll Server
  // We give it some time to process everything (e.g. 10 seconds)
  auto start_time = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(10)) {
    NetworkPacket msg;
    // Poll as many as possible
    while (server->poll(msg)) {
      received_count++;
    }

    if (received_count >= total_expected)
      break;

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cout << "Received " << received_count << " / " << total_expected << " messages." << std::endl;

  // 5. Cleanup
  for (auto &t : client_threads) {
    if (t.joinable())
      t.join();
  }
  server->stop();

  // 6. Assertions
  // Since UDP is unreliable, we might lose some packets even on localhost under stress.
  // But we expect a high success rate. Let's say 90%.
  CHECK(received_count >= total_expected * 0.9);
}
