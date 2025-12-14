/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioServer.cpp
*/

#include "../include/AsioServer.hpp"
#include "../../engineCore/include/ecs/World.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/GunOffset.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/PlayerId.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../engineCore/include/ecs/components/roles/PlayerControlled.hpp"
#include "../include/CapnpHandler.hpp"
#include "ANetworkManager.hpp"
#include "Common.hpp"
#include "network/NetworkPacket.hpp"
#include <array>
#include <asio/bind_executor.hpp>
#include <asio/buffer.hpp>
#include <asio/error.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/ip/udp.hpp>
#include <asio/socket_base.hpp>
#include <asio/strand.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <ostream>
#include <span>
#include <system_error>
#include <thread>

AsioServer::AsioServer(std::uint16_t port)
    : ANetworkManager(std::make_shared<CapnpHandler>()), m_strand(asio::make_strand(m_ioContext)),
      m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), m_nextClientId(0),
      m_workGuard(asio::make_work_guard(m_ioContext))
{
  asio::socket_base::receive_buffer_size option(1024 * 1024 * 8);
  m_socket.set_option(option);
}

AsioServer::~AsioServer()
{
  stop();
}

void AsioServer::start()
{
  std::size_t nbOfThreads = std::thread::hardware_concurrency();
  receive();

  for (std::size_t i = 0; i < nbOfThreads; ++i) {
    m_threadPool.emplace_back([this]() { m_ioContext.run(); });
  }
}

void AsioServer::stop()
{
  m_ioContext.stop();
  m_workGuard.reset();
  for (auto &thread : m_threadPool) {
    if (thread.joinable())
      thread.join();
  }
}

void AsioServer::setWorld(const std::shared_ptr<ecs::World> &world)
{
  m_world = world;
  std::cout << "[Server] World set on network manager" << std::endl;
}

std::size_t AsioServer::getConnectedPlayersCount() const
{
  return m_connectedPlayersCount;
}

std::pair<std::uint32_t, bool> AsioServer::getOrCreateClientId(const asio::ip::udp::endpoint &endpoint)
{
  for (const auto &[id, ep] : m_clients) {
    if (ep == endpoint) {
      return {id, false};
    }
  }
  std::uint32_t clientId = m_nextClientId++;
  m_clients[clientId] = endpoint;
  std::cout << "[Server] New client connected: " << clientId << std::endl;
  return {clientId, true};
}

void AsioServer::send(std::span<const std::byte> data, const std::uint32_t &targetEndpointId)
{
  auto targetEndpointIt = m_clients.find(targetEndpointId);

  if (targetEndpointIt == m_clients.end()) {
    std::cerr << "[Server] Client ID not found: " << targetEndpointId << std::endl;
    return;
  }
  m_socket.async_send_to(
    asio::buffer(data.data(), data.size()), targetEndpointIt->second,
    asio::bind_executor(m_strand, [this, targetEndpointId](const std::error_code &error, std::size_t bytesTransferred) {
      if (error) {
        std::cerr << "[Server] Send error: " << error.message() << std::endl;
      } else {
        // std::cout << "[Server] Sent " << bytesTransferred << " bytes to client " << targetEndpointId << std::endl;
      }
    }));
}

void AsioServer::receive()
{
  auto buffer = std::make_shared<std::array<char, BUFFER_SIZE>>();
  auto senderEndpoint = std::make_shared<asio::ip::udp::endpoint>();

  m_socket.async_receive_from(
    asio::buffer(*buffer), *senderEndpoint,
    asio::bind_executor(
      m_strand, [this, buffer, senderEndpoint](const std::error_code &error, std::size_t bytesTransferred) {
        if (error) {
          if (error != asio::error::operation_aborted) {
            std::cerr << "[Server] Receive error: " << error.message() << std::endl;
            receive();
          }
          return;
        }

        if (bytesTransferred == 0) {
          receive();
          return;
        }

        auto [clientId, isNewClient] = getOrCreateClientId(*senderEndpoint);

        if (isNewClient) {
          ++m_connectedPlayersCount;
          createPlayerEntity(clientId);

          // Handshake: tell the client its assigned id.
          try {
            nlohmann::json hello;
            hello["type"] = "assign_id";
            hello["client_id"] = clientId;
            const std::string jsonStr = hello.dump();
            const auto serialized = getPacketHandler()->serialize(jsonStr);
            send(std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()),
                 clientId);
          } catch (...) {
            // Best-effort handshake
          }
        }

        NetworkPacket message(*buffer, clientId, bytesTransferred);
        m_incomingMessages.push(message);

        receive();
      }));
}

bool AsioServer::poll(NetworkPacket &msg)
{
  return m_incomingMessages.pop(msg);
}

std::unordered_map<std::uint32_t, asio::ip::udp::endpoint> AsioServer::getClients() const
{
  return m_clients;
}

void AsioServer::createPlayerEntity(std::uint32_t clientId)
{
  if (!m_world) {
    std::cerr << "[Server] Cannot create player entity: world not set." << std::endl;
    return;
  }

  ecs::Entity player = m_world->createEntity();

  // Server-authoritative role assignment.
  m_world->addComponent(player, ecs::PlayerControlled{});
  m_world->addComponent(player, ecs::GunOffset{20.0F});

  ecs::Transform transform;
  transform.x = 100.0F;
  transform.y = 300.0F + static_cast<float>(m_connectedPlayersCount) * 50.0F;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  m_world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  m_world->addComponent(player, velocity);

  ecs::Health health;
  health.hp = 100;
  health.maxHp = 100;
  m_world->addComponent(player, health);

  ecs::Input input{};
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  m_world->addComponent(player, input);

  m_world->addComponent(player, ecs::Collider{32.0F, 32.0F});

  // SERVER ASSIGNS VISUAL IDENTITY AS DATA
  // Player sprite decided at creation time
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = 140;  // 350x150 aspect ratio, scaled down 2.5x
  sprite.height = 60;
  m_world->addComponent(player, sprite);

  ecs::Networked networked;
  // Use clientId as the stable network id so the client can address its player.
  networked.networkId = static_cast<ecs::Entity>(clientId);
  m_world->addComponent(player, networked);

  ecs::Score score;
  score.points = 0;
  m_world->addComponent(player, score);

  ecs::PlayerId owner;
  owner.clientId = clientId;
  m_world->addComponent(player, owner);

  std::cout << "[Server] Player entity " << player << " created for client " << clientId << std::endl;
}
