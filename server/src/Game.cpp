/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#include "Game.hpp"
#include "../../engineCore/include/ecs/EngineComponents.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

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

  // Initialize event-based systems
  initializeSystems();

  // Spawn initial player
  // Players are spawned on client connection (see AsioServer::createPlayerEntity).
}

Game::~Game() {}

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

  // SERVER ASSIGNS VISUAL IDENTITY AS DATA
  // Player sprite decided at creation time
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = GameConfig::PLAYER_SPRITE_WIDTH; // 350x150 aspect ratio, scaled down 2.5x
  sprite.height = GameConfig::PLAYER_SPRITE_HEIGHT;
  world->addComponent(player, sprite);

  // Add Networked component for network synchronization
  ecs::Networked networked;
  networked.networkId = player;
  world->addComponent(player, networked);

  // Add Score component for tracking player score
  ecs::Score score;
  score.points = 0;
  world->addComponent(player, score);
}

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

  // SERVER ASSIGNS VISUAL IDENTITY AS DATA
  // Player sprite decided at creation time
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = GameConfig::PLAYER_SPRITE_WIDTH; // 350x150 aspect ratio, scaled down 2.5x
  sprite.height = GameConfig::PLAYER_SPRITE_HEIGHT;
  world->addComponent(player, sprite);

  ecs::Networked networked;
  networked.networkId = static_cast<ecs::Entity>(networkId);
  world->addComponent(player, networked);

  // Add Score component for tracking player score
  ecs::Score score;
  score.points = 0;
  world->addComponent(player, score);
}

void Game::setNetworkManager(const std::shared_ptr<INetworkManager> &networkManager)
{
  m_networkManager = networkManager;
  if (!m_networkManager || !world) {
    return;
  }

  m_networkReceiveSystem = &world->registerSystem<NetworkReceiveSystem>(m_networkManager);
  m_networkSendSystem = &world->registerSystem<NetworkSendSystem>(m_networkManager);
}

void Game::runGameLoop()
{
  constexpr float DELTA_TIME = 0.016F;
  running = true;
  nextTick = std::chrono::steady_clock::now();

  while (running) {
    currentTime = std::chrono::steady_clock::now();

    if (currentTime < nextTick) {
      std::this_thread::sleep_for(nextTick - currentTime);
    }

    // Update all systems
    world->update(DELTA_TIME);

    nextTick += tickRate;
  }
}

std::shared_ptr<ecs::World> Game::getWorld()
{
  return world;
}
