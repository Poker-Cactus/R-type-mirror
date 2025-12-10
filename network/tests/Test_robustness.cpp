/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Test_robustness.cpp
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "AsioClient.hpp"
#include "AsioServer.hpp"
#include <chrono>
#include <doctest/doctest.h>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

TEST_CASE("Server Robustness Test - Garbage Data")
{
  short port = 5002;
  std::shared_ptr<AsioServer> server = std::make_shared<AsioServer>(port);
  server->start();

  auto client = std::make_shared<AsioClient>("127.0.0.1", std::to_string(port));
  client->start();

  // 1. Send Garbage
  std::vector<std::byte> garbage(100);
  for (auto &b : garbage)
    b = static_cast<std::byte>(rand() % 255);

  CHECK_NOTHROW(client->send(garbage, 0));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Server should still be running and able to process valid messages
  auto serialized = client->getPacketHandler()->serialize("PING");
  client->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()),
               0);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  NetworkPacket msg;
  bool received = false;
  // Poll for a bit
  for (int i = 0; i < 20; ++i) {
    if (server->poll(msg)) {
      received = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  CHECK(received == true);

  server->stop();
  client->stop();
}

TEST_CASE("Server Robustness Test - Empty Packet")
{
  short port = 5003;
  std::shared_ptr<AsioServer> server = std::make_shared<AsioServer>(port);
  server->start();

  auto client = std::make_shared<AsioClient>("127.0.0.1", std::to_string(port));
  client->start();

  std::vector<std::byte> empty;
  CHECK_NOTHROW(client->send(empty, 0));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Verify server is alive
  auto serialized = client->getPacketHandler()->serialize("PING");
  client->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()),
               0);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  NetworkPacket msg;
  bool received = false;
  for (int i = 0; i < 20; ++i) {
    if (server->poll(msg)) {
      received = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  CHECK(received == true);

  server->stop();
  client->stop();
}

TEST_CASE("Client Robustness Test - Garbage Data from Server")
{
  short port = 5004;
  std::shared_ptr<AsioServer> server = std::make_shared<AsioServer>(port);
  server->start();

  auto client = std::make_shared<AsioClient>("127.0.0.1", std::to_string(port));
  client->start();

  // Client sends a message first so Server knows about it (and assigns ID)
  auto serialized = client->getPacketHandler()->serialize("HELLO");
  client->send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()),
               0);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  NetworkPacket msg;
  bool serverReceived = false;
  for (int i = 0; i < 20; ++i) {
    if (server->poll(msg)) {
      serverReceived = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  REQUIRE(serverReceived == true);

  uint32_t clientId = msg.getSenderEndpointId();

  // Server sends garbage to client
  std::vector<std::byte> garbage(100);
  for (auto &b : garbage)
    b = static_cast<std::byte>(rand() % 255);

  CHECK_NOTHROW(server->send(garbage, clientId));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Client should still be alive.
  // Send valid data from server to client
  auto valid = client->getPacketHandler()->serialize("PONG");
  std::vector<std::byte> validBytesVec;
  for (char c : valid)
    validBytesVec.push_back(static_cast<std::byte>(c));

  server->send(validBytesVec, clientId);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  NetworkPacket clientMsg;
  bool clientReceived = false;
  for (int i = 0; i < 20; ++i) {
    if (client->poll(clientMsg)) {
      clientReceived = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  CHECK(clientReceived == true);

  server->stop();
  client->stop();
}
