/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game - Server-side game logic and ECS system management
*/

#include "Game.hpp"
#include "../../engineCore/include/ecs/EngineComponents.hpp"
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

  world->addComponent(player, ecs::PlayerControlled{});
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
  health.hp = GameConfig::PLAYER_MAX_HP;
  health.maxHp = GameConfig::PLAYER_MAX_HP;
  world->addComponent(player, health);

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

  world->addComponent(player, ecs::PlayerControlled{});
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
  health.hp = GameConfig::PLAYER_MAX_HP;
  health.maxHp = GameConfig::PLAYER_MAX_HP;
  world->addComponent(player, health);

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
  networked.networkId = static_cast<ecs::Entity>(networkId);
  world->addComponent(player, networked);

  ecs::Score score;
  score.points = 0;
  world->addComponent(player, score);
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
}

/**
 * @brief Main game loop with variable delta time calculation
 *
 * Runs continuously until stopped, updating all systems with the actual
 * elapsed time between frames. Uses a fixed tick rate for scheduling but
 * calculates real delta time for physics accuracy. Sleeps if running ahead
 * of schedule to maintain consistent tick rate.
 *
 * The game loop only updates systems after a player has sent the ready message.
 */
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

    if (gameStarted) {
      world->update(deltaTime);
    }

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

/**
 * @brief Starts the game and enables enemy spawning
 *
 * Called when a player sends a ready message. Activates the spawn system
 * to begin spawning enemies.
 */
void Game::startGame()
{
  gameStarted = true;
}

/**
 * @brief Checks if the game has been started
 *
 * @return true if a player has sent the ready signal, false otherwise
 */
bool Game::isGameStarted() const
{
  return gameStarted;
}
