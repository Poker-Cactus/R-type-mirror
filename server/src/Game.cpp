/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Game
*/

#include "Game.hpp"
#include "../../engineCore/include/ecs/components/Collider.hpp"
#include "../../engineCore/include/ecs/components/Health.hpp"
#include "../../engineCore/include/ecs/components/Input.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include "../../engineCore/include/ecs/components/Velocity.hpp"
#include "ecs/Entity.hpp"
#include "ecs/World.hpp"
#include "ecs/systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/EnemyAISystem.hpp"
#include "systems/LifetimeSystem.hpp"
#include "systems/ShootingSystem.hpp"
#include "systems/SpawnSystem.hpp"
#include <chrono>
#include <memory>
#include <thread>

Game::Game()
{
  world = std::make_shared<ecs::World>();

  // Register all systems
  world->registerSystem<ecs::MovementSystem>();
  world->registerSystem<server::CollisionSystem>();

  damageSystem = &world->registerSystem<server::DamageSystem>();
  deathSystem = &world->registerSystem<server::DeathSystem>();
  shootingSystem = &world->registerSystem<server::ShootingSystem>();

  world->registerSystem<server::EnemyAISystem>();

  spawnSystem = &world->registerSystem<server::SpawnSystem>();
  world->registerSystem<server::LifetimeSystem>();

  // Initialize event-based systems
  initializeSystems();

  // Spawn initial player
  spawnPlayer();
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
  if (spawnSystem != nullptr) {
    spawnSystem->initialize(*world);
  }
}

void Game::spawnPlayer()
{
  ecs::Entity player = world->createEntity();

  ecs::Transform transform;
  transform.x = 100.0F;
  transform.y = 300.0F;
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  world->addComponent(player, transform);

  ecs::Velocity velocity;
  velocity.dx = 0.0F;
  velocity.dy = 0.0F;
  world->addComponent(player, velocity);

  ecs::Health health;
  health.hp = 100;
  health.maxHp = 100;
  world->addComponent(player, health);

  ecs::Input input;
  input.up = false;
  input.down = false;
  input.left = false;
  input.right = false;
  input.shoot = false;
  world->addComponent(player, input);

  world->addComponent(player, ecs::Collider{32.0F, 32.0F});
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
