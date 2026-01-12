/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game - Server-side game logic and ECS system management
*/

#include "Game.hpp"
#include "../../engineCore/include/ecs/EngineComponents.hpp"
#include "systems/SpawnSystem.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

/**
 * @brief Constructs the game and initializes all ECS systems
 *
 * Registers all game systems including movement, collision, damage, death,
 * shooting, score, enemy AI, spawn, and lifetime systems. Event-based systems
 * are initialized after registration.
 */
Game::Game()
{
  world = std::make_shared<ecs::World>();

  // Register all systems
  world->registerSystem<server::InputMovementSystem>();
  world->registerSystem<ecs::MovementSystem>();
  world->registerSystem<server::CollisionSystem>();

  damageSystem = &world->registerSystem<server::DamageSystem>();
  deathSystem = &world->registerSystem<server::DeathSystem>();
  shootingSystem = &world->registerSystem<server::ShootingSystem>();
  scoreSystem = &world->registerSystem<server::ScoreSystem>();

  world->registerSystem<server::EnemyAISystem>();

  spawnSystem = &world->registerSystem<server::SpawnSystem>();
  world->registerSystem<server::EntityLifetimeSystem>();
  world->registerSystem<server::LifetimeSystem>();

  initializeSystems();
}

Game::~Game() {}

/**
 * @brief Initializes event-based systems that require explicit setup
 *
 * Calls initialize() on systems that need to register event handlers
 * or perform additional setup after registration.
 */
void Game::initializeSystems()
{
  if (damageSystem != nullptr) {
    damageSystem->initialize(*world);
  }
  if (deathSystem != nullptr) {
    deathSystem->initialize(*world);
  }
  if (shootingSystem != nullptr) {
    shootingSystem->initialize(*world);
  }
  if (scoreSystem != nullptr) {
    scoreSystem->initialize(*world);
  }
  if (spawnSystem != nullptr) {
    spawnSystem->initialize(*world);
  }
}

/**
 * @brief Spawns a player entity with default configuration
 *
 * Creates a new player entity with all required components including
 * transform, velocity, health, input, collider, sprite, networking, and score.
 * Uses configuration values from GameConfig namespace.
 */
void Game::spawnPlayer()
{
  ecs::Entity player = world->createEntity();

  world->addComponent(player, ecs::GunOffset{GameConfig::PLAYER_GUN_OFFSET});

  ecs::Transform transform;
  transform.x = GameConfig::PLAYER_SPAWN_X;
  transform.y = GameConfig::PLAYER_SPAWN_Y;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  world->addComponent(player, velocity);

  ecs::Health health;
  int baseHealth = GameConfig::PLAYER_MAX_HP;

  // Adjust player health based on difficulty
  switch (currentDifficulty) {
  case Difficulty::EASY:
    baseHealth = static_cast<int>(baseHealth * 1.5f); // 150 HP
    break;
  case Difficulty::MEDIUM:
    // Normal health (100 HP)
    break;
  case Difficulty::EXPERT:
    baseHealth = static_cast<int>(baseHealth * 0.75f); // 75 HP
    break;
  }

  health.hp = baseHealth;
  health.maxHp = baseHealth;
  world->addComponent(player, health);

  std::cout << "[Server] Spawning player with " << baseHealth << " HP (difficulty: "
            << (currentDifficulty == Difficulty::EASY       ? "easy"
                  : currentDifficulty == Difficulty::MEDIUM ? "medium"
                                                            : "expert")
            << ")" << std::endl;

  ecs::Input input;
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  world->addComponent(player, input);

  world->addComponent(player, ecs::Collider{GameConfig::PLAYER_COLLIDER_SIZE, GameConfig::PLAYER_COLLIDER_SIZE});

  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = GameConfig::PLAYER_SPRITE_WIDTH;
  sprite.height = GameConfig::PLAYER_SPRITE_HEIGHT;
  world->addComponent(player, sprite);

  ecs::Networked networked;
  networked.networkId = player;
  world->addComponent(player, networked);

  ecs::Score score;
  score.points = 0;
  world->addComponent(player, score);
}

/**
 * @brief Spawns a player entity with a specific network ID
 *
 * @param networkId The network identifier assigned by the server for this client
 *
 * Creates a player entity similar to spawnPlayer() but associates it with
 * a specific network client ID for multiplayer synchronization.
 */
void Game::spawnPlayer(std::uint32_t networkId)
{
  ecs::Entity player = world->createEntity();

  world->addComponent(player, ecs::GunOffset{GameConfig::PLAYER_GUN_OFFSET});

  ecs::Transform transform;
  transform.x = GameConfig::PLAYER_SPAWN_X;
  transform.y = GameConfig::PLAYER_SPAWN_Y;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  world->addComponent(player, velocity);

  ecs::Health health;
  int baseHealth = GameConfig::PLAYER_MAX_HP;

  // Adjust player health based on difficulty
  switch (currentDifficulty) {
  case Difficulty::EASY:
    baseHealth = static_cast<int>(baseHealth * 1.5f); // 150 HP
    break;
  case Difficulty::MEDIUM:
    // Normal health (100 HP)
    break;
  case Difficulty::EXPERT:
    baseHealth = static_cast<int>(baseHealth * 0.75f); // 75 HP
    break;
  }

  health.hp = baseHealth;
  health.maxHp = baseHealth;
  world->addComponent(player, health);

  std::cout << "[Server] Spawning player with networkId " << networkId << " with " << baseHealth << " HP (difficulty: "
            << (currentDifficulty == Difficulty::EASY       ? "easy"
                  : currentDifficulty == Difficulty::MEDIUM ? "medium"
                                                            : "expert")
            << ")" << std::endl;

  ecs::Input input;
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  world->addComponent(player, input);

  world->addComponent(player, ecs::Collider{GameConfig::PLAYER_COLLIDER_SIZE, GameConfig::PLAYER_COLLIDER_SIZE});

  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = GameConfig::PLAYER_SPRITE_WIDTH;
  sprite.height = GameConfig::PLAYER_SPRITE_HEIGHT;
  world->addComponent(player, sprite);

  ecs::Networked networked;
  // Use the created entity id as the network id (unique within this world).
  networked.networkId = player;
  world->addComponent(player, networked);

  ecs::Score score;
  score.points = 0;
  world->addComponent(player, score);

  // Add PlayerId so server can route inputs to this player
  ecs::PlayerId playerId;
  playerId.clientId = networkId;
  world->addComponent(player, playerId);

  std::cout << "[Server] Spawned player entity " << player << " for client " << networkId << '\n';
}

/**
 * @brief Sets the network manager for multiplayer support
 *
 * @param networkManager Shared pointer to the network manager instance
 *
 * Registers network receive and send systems with the provided network manager.
 * Returns early if the network manager or world is not available.
 */
void Game::setNetworkManager(const std::shared_ptr<INetworkManager> &networkManager)
{
  m_networkManager = networkManager;
  if (!m_networkManager || !world) {
    return;
  }

  m_networkReceiveSystem = &world->registerSystem<NetworkReceiveSystem>(m_networkManager);
  m_networkSendSystem = &world->registerSystem<NetworkSendSystem>(m_networkManager);

  if (m_networkReceiveSystem != nullptr) {
    m_networkReceiveSystem->setGame(this);
  }

  if (m_networkSendSystem != nullptr) {
    m_networkSendSystem->setLobbyManager(&m_lobbyManager);
  }
  // Give the lobby manager access to the network manager so lobbies can send direct messages
  m_lobbyManager.setNetworkManager(m_networkManager);
}

void Game::runGameLoop()
{
  running = true;
  nextTick = std::chrono::steady_clock::now();
  auto lastUpdateTime = std::chrono::steady_clock::now();

  while (running) {
    currentTime = std::chrono::steady_clock::now();

    if (currentTime < nextTick) {
      std::this_thread::sleep_for(nextTick - currentTime);
      continue;
    }

    auto deltaTimeDuration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastUpdateTime);
    float deltaTime = static_cast<float>(deltaTimeDuration.count()) / GameConfig::MICROSECONDS_TO_SECONDS;
    lastUpdateTime = currentTime;

    // Always process incoming network messages (uses main world for system registration,
    // but routes to lobby worlds internally)
    if (m_networkReceiveSystem != nullptr) {
      m_networkReceiveSystem->update(*world, deltaTime);
    }

    // Update each active lobby's game world independently
    for (const auto &[code, lobby] : m_lobbyManager.getLobbies()) {
      if (lobby && lobby->isGameStarted() && !lobby->isEmpty()) {
        // Update this lobby's isolated world
        lobby->update(deltaTime);
      }
    }

    // Send snapshots for each lobby (NetworkSendSystem now handles per-lobby sending)
    if (m_networkSendSystem != nullptr) {
      m_networkSendSystem->update(*world, deltaTime);
    }

    // Clean up empty lobbies at end of frame (safe after all systems updated)
    m_lobbyManager.cleanupEmptyLobbies();

    nextTick += tickRate;
  }
}

/**
 * @brief Returns the ECS world instance
 *
 * @return Shared pointer to the world containing all entities and components
 */
std::shared_ptr<ecs::World> Game::getWorld()
{
  return world;
}

void Game::startGame()
{
  std::cout << "[Server] Starting game with " << m_lobbyClients.size() << " players" << '\n';
  gameStarted = true;
  world->getSystem<server::SpawnSystem>()->difficulty = currentDifficulty;
}

bool Game::isGameStarted() const
{
  return gameStarted;
}

void Game::addClientToLobby(std::uint32_t clientId)
{
  m_lobbyClients.insert(clientId);
  std::cout << "[Server] Player " << clientId << " joined the lobby (" << m_lobbyClients.size() << " players waiting)"
            << '\n';
}

void Game::removeClientFromLobby(std::uint32_t clientId)
{
  m_lobbyClients.erase(clientId);
  std::cout << "[Server] Player " << clientId << " left the lobby (" << m_lobbyClients.size() << " players remaining)"
            << '\n';
}

const std::unordered_set<std::uint32_t> &Game::getLobbyClients() const
{
  return m_lobbyClients;
}

LobbyManager &Game::getLobbyManager()
{
  return m_lobbyManager;
}
