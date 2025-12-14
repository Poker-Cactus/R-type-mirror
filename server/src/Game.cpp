/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#include "Game.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/GunOffset.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/Networked.hpp"
#include "../../engineCore/include/ecs/components/Score.hpp"
#include "../../engineCore/include/ecs/components/Sprite.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../engineCore/include/ecs/components/Velocity.hpp"
#include "../../engineCore/include/ecs/components/roles/PlayerControlled.hpp"
#include "ecs/Entity.hpp"
#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/EnemyAISystem.hpp"
#include "systems/EntityLifetimeSystem.hpp"
#include "systems/InputMovementSystem.hpp"
#include "systems/LifetimeSystem.hpp"
#include "systems/NetworkReceiveSystem.hpp"
#include "systems/NetworkSendSystem.hpp"
#include "systems/ScoreSystem.hpp"
#include "systems/ShootingSystem.hpp"
#include "systems/SpawnSystem.hpp"
#include <chrono>
#include <memory>
#include <thread>

// Player spawn configuration constants
namespace {
  constexpr float PLAYER_GUN_OFFSET = 20.0F;
  constexpr float PLAYER_START_X = 100.0F;
  constexpr float PLAYER_START_Y = 300.0F;
  constexpr int PLAYER_INITIAL_HEALTH = 100;
  constexpr float PLAYER_COLLIDER_SIZE = 32.0F;
  constexpr unsigned int PLAYER_SPRITE_WIDTH = 140;
  constexpr unsigned int PLAYER_SPRITE_HEIGHT = 60;
}

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
  world->addComponent(player, ecs::GunOffset{PLAYER_GUN_OFFSET});

  ecs::Transform transform;
  transform.x = PLAYER_START_X;
  transform.y = PLAYER_START_Y;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  world->addComponent(player, velocity);

  ecs::Health health;
  health.hp = PLAYER_INITIAL_HEALTH;
  health.maxHp = PLAYER_INITIAL_HEALTH;
  world->addComponent(player, health);

  ecs::Input input;
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  world->addComponent(player, input);

  world->addComponent(player, ecs::Collider{PLAYER_COLLIDER_SIZE, PLAYER_COLLIDER_SIZE});

  // SERVER ASSIGNS VISUAL IDENTITY AS DATA
  // Player sprite decided at creation time
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = PLAYER_SPRITE_WIDTH; // 350x150 aspect ratio, scaled down 2.5x
  sprite.height = PLAYER_SPRITE_HEIGHT;
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
  world->addComponent(player, ecs::GunOffset{PLAYER_GUN_OFFSET});

  ecs::Transform transform;
  transform.x = PLAYER_START_X;
  transform.y = PLAYER_START_Y;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  world->addComponent(player, velocity);

  ecs::Health health;
  health.hp = PLAYER_INITIAL_HEALTH;
  health.maxHp = PLAYER_INITIAL_HEALTH;
  world->addComponent(player, health);

  ecs::Input input;
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  world->addComponent(player, input);

  world->addComponent(player, ecs::Collider{PLAYER_COLLIDER_SIZE, PLAYER_COLLIDER_SIZE});

  // SERVER ASSIGNS VISUAL IDENTITY AS DATA
  // Player sprite decided at creation time
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = PLAYER_SPRITE_WIDTH; // 350x150 aspect ratio, scaled down 2.5x
  sprite.height = PLAYER_SPRITE_HEIGHT;
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
  running = true;
  nextTick = std::chrono::steady_clock::now();

  while (running) {
    currentTime = std::chrono::steady_clock::now();

    if (currentTime < nextTick) {
      std::this_thread::sleep_for(nextTick - currentTime);
    }

    // Update all systems
    world->update(0.016F);

    nextTick += tickRate;
  }
}

std::shared_ptr<ecs::World> Game::getWorld()
{
  return world;
}
