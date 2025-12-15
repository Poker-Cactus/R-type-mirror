/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AsioServer.cpp
*/

#include "../include/AsioServer.hpp"
#include "../../engineCore/include/ecs/EngineComponents.hpp"
#include "../include/CapnpHandler.hpp"
#include "../include/NetworkConfig.hpp"
#include "ANetworkManager.hpp"
#include "Common.hpp"
#include "network/NetworkPacket.hpp"
#include <array>
#include <cstddef>
#include <iostream>
#include <nlohmann/json.hpp>
#include <system_error>

AsioServer::AsioServer(std::uint16_t port)
    : ANetworkManager(std::make_shared<CapnpHandler>()), m_strand(asio::make_strand(m_ioContext)),
      m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), m_nextClientId(0),
      m_workGuard(asio::make_work_guard(m_ioContext))
{
  asio::socket_base::receive_buffer_size option(NetworkConfig::RECEIVE_BUFFER_SIZE_KB *
                                                NetworkConfig::RECEIVE_BUFFER_SIZE_KB *
                                                NetworkConfig::RECEIVE_BUFFER_MULTIPLIER);
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
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void AsioServer::setWorld(const std::shared_ptr<ecs::World> &world)
{
  m_world = world;
  std::cout << "[Server] World set on network manager" << '\n';
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
  std::cout << "[Server] New client connected: " << clientId << '\n';
  return {clientId, true};
}

void AsioServer::send(std::span<const std::byte> data, const std::uint32_t &targetEndpointId)
{
  auto targetEndpointIt = m_clients.find(targetEndpointId);

  if (targetEndpointIt == m_clients.end()) {
    std::cerr << "[Server] Client ID not found: " << targetEndpointId << '\n';
    return;
  }
  m_socket.async_send_to(
    asio::buffer(data.data(), data.size()), targetEndpointIt->second,
    asio::bind_executor(m_strand, [this, targetEndpointId](const std::error_code &error, std::size_t bytesTransferred) {
      if (error) {
        std::cerr << "[Server] Send error: " << error.message() << '\n';
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
            std::cerr << "[Server] Receive error: " << error.message() << '\n';
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
          } catch ([[maybe_unused]] const std::exception &e) { // NOLINT(bugprone-empty-catch)
            // Best-effort handshake - silent failure acceptable for non-critical handshake
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
    std::cerr << "[Server] Cannot create player entity: world not set." << '\n';
    return;
  }

  ecs::Entity player = m_world->createEntity();

  // Server-authoritative role assignment.
  m_world->addComponent(player, ecs::PlayerControlled{});
  m_world->addComponent(player, ecs::GunOffset{NetworkConfig::PLAYER_GUN_OFFSET});

  ecs::Transform transform;
  transform.x = NetworkConfig::PLAYER_SPAWN_X;
  transform.y =
    NetworkConfig::PLAYER_SPAWN_Y + static_cast<float>(m_connectedPlayersCount) * NetworkConfig::PLAYER_SPAWN_Y_OFFSET;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  m_world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  m_world->addComponent(player, velocity);

  ecs::Health health;
  health.hp = NetworkConfig::PLAYER_MAX_HP;
  health.maxHp = NetworkConfig::PLAYER_MAX_HP;
  m_world->addComponent(player, health);

  ecs::Input input{};
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  m_world->addComponent(player, input);

  m_world->addComponent(player,
                        ecs::Collider{NetworkConfig::PLAYER_COLLIDER_SIZE, NetworkConfig::PLAYER_COLLIDER_SIZE});

  // SERVER ASSIGNS VISUAL IDENTITY AS DATA
  // Player sprite decided at creation time
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = NetworkConfig::PLAYER_SPRITE_WIDTH; // 350x150 aspect ratio, scaled down 2.5x
  sprite.height = NetworkConfig::PLAYER_SPRITE_HEIGHT;
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

  std::cout << "[Server] Player entity " << player << " created for client " << clientId << '\n';
}
