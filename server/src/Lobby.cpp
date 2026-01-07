/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Lobby implementation - Manages isolated game worlds per lobby
*/

#include "Lobby.hpp"
#include "../../engineCore/include/ecs/EngineComponents.hpp"
#include "../../network/include/INetworkManager.hpp"
#include "../include/Game.hpp"
#include "../include/ServerSystems.hpp"
#include "WorldLobbyRegistry.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

Lobby::Lobby(const std::string &code, std::shared_ptr<INetworkManager> networkManager)
    : m_code(code), m_networkManager(std::move(networkManager))
{
  // Create isolated world for this lobby
  m_world = std::make_shared<ecs::World>();
  std::cout << "[Lobby:" << m_code << "] Created isolated game world" << '\n';

  // Register this world -> lobby mapping for systems that need to find the lobby
  // (DeathSystem will use Lobby::getLobbyForWorld via this global mapping)
  registerWorldLobbyMapping(m_world.get(), this);
}

Lobby::~Lobby()
{
  std::cout << "[Lobby:" << m_code << "] Destroying lobby..." << '\n';

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

  std::cout << "[Lobby:" << m_code << "] Destroyed" << '\n';
  unregisterWorldLobbyMapping(m_world.get());
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

  std::cout << "[Lobby:" << m_code << "] Game started with " << m_clients.size() << " players" << '\n';
}

void Lobby::stopGame()
{
  if (!m_gameStarted) {
    return;
  }

  std::cout << "[Lobby:" << m_code << "] Stopping game..." << '\n';

  m_gameStarted = false;

  if (!m_world) {
    m_playerEntities.clear();
    std::cout << "[Lobby:" << m_code << "] Game stopped (no world)" << '\n';
    return;
  }

  // 1) Remove all systems so they can unsubscribe from events and release resources
  m_world->clearSystems();

  // 2) Clear any event listeners on the world's event bus
  m_world->getEventBus().clear();

  // 3) Destroy all entities to ensure components are removed and no stale state remains
  {
    ecs::ComponentSignature emptySig; // matches all entities
    std::vector<ecs::Entity> allEntities;
    m_world->getEntitiesWithSignature(emptySig, allEntities);
    for (auto ent : allEntities) {
      if (m_world->isAlive(ent)) {
        m_world->destroyEntity(ent);
      }
    }
  }

  // 4) Clear player entity tracking
  m_playerEntities.clear();

  // 5) Optionally replace the world to ensure a fully fresh state for the next start
  //    This guarantees no lingering references remain in other subsystems.
  m_world = std::make_shared<ecs::World>();

  std::cout << "[Lobby:" << m_code << "] Game stopped" << '\n';
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
  auto player_entity_it = m_playerEntities.find(clientId);
  if (player_entity_it != m_playerEntities.end()) {
    return player_entity_it->second;
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
    spawnSystem->difficulty = static_cast<Difficulty>(m_difficulty);
  }

  std::cout << "[Lobby:" << m_code << "] Initialized game systems" << '\n';
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
  constexpr int PLAYER_VERTICAL_SPACING = 80;
  // Offset Y position based on number of players to avoid overlap
  transform.y = GameConfig::PLAYER_SPAWN_Y + static_cast<float>(m_playerEntities.size() * PLAYER_VERTICAL_SPACING);
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  m_world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  m_world->addComponent(player, velocity);

  // Apply difficulty-based HP
  int startingHP = GameConfig::getPlayerHPForDifficulty(m_difficulty);
  ecs::Health health;
  health.hp = startingHP;
  health.maxHp = startingHP;
  m_world->addComponent(player, health);

  std::cout << "[Lobby:" << m_code << "] Player " << clientId << " spawned with " << startingHP
            << " HP (Difficulty: "
            << (m_difficulty == GameConfig::Difficulty::EASY
                  ? "EASY"
                  : (m_difficulty == GameConfig::Difficulty::MEDIUM ? "MEDIUM" : "EXPERT"))
            << ")" << '\n';

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
  // Use the entity id as the authoritative network id to avoid collisions
  // with other entities (enemies/projectiles) that also use entity ids.
  networked.networkId = player;
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

  std::cout << "[Lobby:" << m_code << "] Spawned player entity " << player << " for client " << clientId << '\n';
}

void Lobby::destroyPlayerEntity(std::uint32_t clientId)
{
  auto player_entity_it = m_playerEntities.find(clientId);
  if (player_entity_it == m_playerEntities.end()) {
    return; // No entity for this client
  }

  // Safely destroy the entity if world is valid and entity is alive
  if (m_world && m_world->isAlive(player_entity_it->second)) {
    m_world->destroyEntity(player_entity_it->second);
    std::cout << "[Lobby:" << m_code << "] Destroyed player entity " << player_entity_it->second << " for client "
              << clientId << '\n';
  }

  // Always remove from tracking map
  m_playerEntities.erase(player_entity_it);
}

void Lobby::sendJsonToClient(std::uint32_t clientId, const nlohmann::json &message) const
{
  if (!m_networkManager) {
    return;
  }
  const std::string jsonStr = message.dump();
  const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);
  m_networkManager->send(
    std::span<const std::byte>(reinterpret_cast<const std::byte *>(serialized.data()), serialized.size()), clientId);
}

void Lobby::setDifficulty(GameConfig::Difficulty difficulty)
{
  if (m_gameStarted) {
    std::cerr << "[Lobby:" << m_code << "] Cannot change difficulty after game has started" << '\n';
    return;
  }
  m_difficulty = difficulty;
  std::cout << "[Lobby:" << m_code << "] Difficulty set to "
            << (difficulty == GameConfig::Difficulty::EASY
                  ? "EASY (150 HP)"
                  : (difficulty == GameConfig::Difficulty::MEDIUM ? "MEDIUM (100 HP)" : "EXPERT (75 HP)"))
            << '\n';
}

GameConfig::Difficulty Lobby::getDifficulty() const { return m_difficulty; }
