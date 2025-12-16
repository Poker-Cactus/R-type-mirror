/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Lobby implementation - Manages isolated game worlds per lobby
*/

#include "Lobby.hpp"
#include "../../engineCore/include/ecs/EngineComponents.hpp"
#include "../include/Game.hpp"
#include "../include/ServerSystems.hpp"
#include <iostream>

Lobby::Lobby(const std::string &code) : m_code(code)
{
  // Create isolated world for this lobby
  m_world = std::make_shared<ecs::World>();
  std::cout << "[Lobby:" << m_code << "] Created isolated game world" << std::endl;
}

Lobby::~Lobby()
{
  std::cout << "[Lobby:" << m_code << "] Destroying lobby..." << std::endl;

  // Ensure game is stopped before destruction
  if (m_gameStarted) {
    stopGame();
  }

  // Clear all client tracking
  m_clients.clear();
  m_playerEntities.clear();

  // Clear world systems and entities before shared_ptr destruction
  if (m_world) {
    m_world->clearSystems();
    // Note: Entity cleanup is handled by World's own destruction
  }

  std::cout << "[Lobby:" << m_code << "] Destroyed" << std::endl;
}

bool Lobby::addClient(std::uint32_t clientId)
{
  auto [_, inserted] = m_clients.insert(clientId);
  if (inserted) {
    std::cout << "[Lobby:" << m_code << "] Player " << clientId << " joined (" << m_clients.size() << " players)"
              << '\n';
  }
  return inserted;
}

bool Lobby::removeClient(std::uint32_t clientId)
{
  // Destroy the player entity if it exists
  destroyPlayerEntity(clientId);

  auto removed = m_clients.erase(clientId) > 0;
  if (removed) {
    std::cout << "[Lobby:" << m_code << "] Player " << clientId << " left (" << m_clients.size() << " remaining)"
              << '\n';
  }
  return removed;
}

bool Lobby::isEmpty() const
{
  return m_clients.empty();
}

std::size_t Lobby::getClientCount() const
{
  return m_clients.size();
}

const std::string &Lobby::getCode() const
{
  return m_code;
}

const std::unordered_set<std::uint32_t> &Lobby::getClients() const
{
  return m_clients;
}

bool Lobby::hasClient(std::uint32_t clientId) const
{
  return m_clients.find(clientId) != m_clients.end();
}

void Lobby::startGame()
{
  if (m_gameStarted) {
    return;
  }

  m_gameStarted = true;

  // Initialize systems for this lobby's world
  initializeSystems();

  // Spawn player entities for all clients in this lobby
  for (std::uint32_t clientId : m_clients) {
    spawnPlayer(clientId);
  }

  std::cout << "[Lobby:" << m_code << "] Game started with " << m_clients.size() << " players" << std::endl;
}

void Lobby::stopGame()
{
  if (!m_gameStarted) {
    return;
  }

  std::cout << "[Lobby:" << m_code << "] Stopping game..." << std::endl;

  m_gameStarted = false;

  // Destroy all player entities
  if (m_world) {
    for (const auto &[clientId, entity] : m_playerEntities) {
      if (m_world->isAlive(entity)) {
        m_world->destroyEntity(entity);
      }
    }
  }

  // Clear player entity tracking
  m_playerEntities.clear();

  std::cout << "[Lobby:" << m_code << "] Game stopped" << std::endl;
}

bool Lobby::isGameStarted() const
{
  return m_gameStarted;
}

std::shared_ptr<ecs::World> Lobby::getWorld() const
{
  return m_world;
}

void Lobby::update(float deltaTime)
{
  if (m_gameStarted && m_world) {
    m_world->update(deltaTime);
  }
}

ecs::Entity Lobby::getPlayerEntity(std::uint32_t clientId) const
{
  auto it = m_playerEntities.find(clientId);
  if (it != m_playerEntities.end()) {
    return it->second;
  }
  return 0;
}

void Lobby::initializeSystems()
{
  if (!m_world) {
    return;
  }

  // Register all game systems for this lobby's world
  m_world->registerSystem<server::InputMovementSystem>();
  m_world->registerSystem<ecs::MovementSystem>();
  m_world->registerSystem<server::CollisionSystem>();

  auto *damageSystem = &m_world->registerSystem<server::DamageSystem>();
  auto *deathSystem = &m_world->registerSystem<server::DeathSystem>();
  auto *shootingSystem = &m_world->registerSystem<server::ShootingSystem>();
  auto *scoreSystem = &m_world->registerSystem<server::ScoreSystem>();

  m_world->registerSystem<server::EnemyAISystem>();

  auto *spawnSystem = &m_world->registerSystem<server::SpawnSystem>();
  m_world->registerSystem<server::EntityLifetimeSystem>();
  m_world->registerSystem<server::LifetimeSystem>();

  // Initialize event-based systems
  if (damageSystem != nullptr) {
    damageSystem->initialize(*m_world);
  }
  if (deathSystem != nullptr) {
    deathSystem->initialize(*m_world);
  }
  if (shootingSystem != nullptr) {
    shootingSystem->initialize(*m_world);
  }
  if (scoreSystem != nullptr) {
    scoreSystem->initialize(*m_world);
  }
  if (spawnSystem != nullptr) {
    spawnSystem->initialize(*m_world);
  }

  std::cout << "[Lobby:" << m_code << "] Initialized game systems" << std::endl;
}

void Lobby::spawnPlayer(std::uint32_t clientId)
{
  if (!m_world) {
    return;
  }

  ecs::Entity player = m_world->createEntity();

  m_world->addComponent(player, ecs::PlayerControlled{});
  m_world->addComponent(player, ecs::GunOffset{GameConfig::PLAYER_GUN_OFFSET});

  ecs::Transform transform;
  transform.x = GameConfig::PLAYER_SPAWN_X;
  // Offset Y position based on number of players to avoid overlap
  transform.y = GameConfig::PLAYER_SPAWN_Y + static_cast<float>(m_playerEntities.size() * 80);
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  m_world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  m_world->addComponent(player, velocity);

  ecs::Health health;
  health.hp = GameConfig::PLAYER_MAX_HP;
  health.maxHp = GameConfig::PLAYER_MAX_HP;
  m_world->addComponent(player, health);

  ecs::Input input;
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  m_world->addComponent(player, input);

  m_world->addComponent(player, ecs::Collider{GameConfig::PLAYER_COLLIDER_SIZE, GameConfig::PLAYER_COLLIDER_SIZE});

  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = GameConfig::PLAYER_SPRITE_WIDTH;
  sprite.height = GameConfig::PLAYER_SPRITE_HEIGHT;
  m_world->addComponent(player, sprite);

  ecs::Networked networked;
  networked.networkId = static_cast<ecs::Entity>(clientId);
  m_world->addComponent(player, networked);

  ecs::Score score;
  score.points = 0;
  m_world->addComponent(player, score);

  // Add PlayerId so server can route inputs to this player
  ecs::PlayerId playerId;
  playerId.clientId = clientId;
  m_world->addComponent(player, playerId);

  // Track the player entity
  m_playerEntities[clientId] = player;

  std::cout << "[Lobby:" << m_code << "] Spawned player entity " << player << " for client " << clientId << std::endl;
}

void Lobby::destroyPlayerEntity(std::uint32_t clientId)
{
  auto it = m_playerEntities.find(clientId);
  if (it == m_playerEntities.end()) {
    return; // No entity for this client
  }

  // Safely destroy the entity if world is valid and entity is alive
  if (m_world && m_world->isAlive(it->second)) {
    m_world->destroyEntity(it->second);
    std::cout << "[Lobby:" << m_code << "] Destroyed player entity " << it->second << " for client " << clientId
              << std::endl;
  }

  // Always remove from tracking map
  m_playerEntities.erase(it);
}
