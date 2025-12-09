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
    short port = 5000;

    std::cout << "Starting Server on port " << port << "..." << std::endl;

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
    int msgs_per_thread = 50;
    int total_expected = num_threads * msgs_per_thread;

    std::vector<std::thread> client_threads;

    // Barrier to start all threads roughly at same time
    std::atomic<bool> start_flag(false);

    std::cout << "Spawning " << num_threads << " threads sending " << msgs_per_thread << " messages each..."
              << std::endl;

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
                    auto serialized = client->getPacketHandler()->serialize("PING");
                    client->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()),
                                                            serialized.size()),
                                 0);

                    // Small sleep to avoid overwhelming local UDP buffer too fast
                    std::this_thread::sleep_for(std::chrono::microseconds(500));
                }

                // Keep client alive for a bit to ensure sends go out
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                client->stop();
            } catch (const std::exception &e) {
                std::cerr << "Client thread error: " << e.what() << std::endl;
            }
        });
    }

    // 3. Start the load
    start_flag = true;

    // 4. Poll Server
    // We give it some time to process everything
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(5)) {
        NetworkPacket msg;
        while (server->poll(msg)) {
            received_count++;
        }
        if (received_count >= total_expected)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Received " << received_count << " / " << total_expected << " messages." << std::endl;

    // 5. Cleanup
    for (auto &t : client_threads) {
        if (t.joinable())
            t.join();
    }
    server->stop();

    // 6. Assertions
    // We allow some packet loss in UDP, but locally it should be minimal.
    // Let's require at least 90% success to be robust against minor OS buffer drops.
    CHECK(received_count >= total_expected * 0.9);
    CHECK(received_count <= total_expected); // Should not receive more
}
