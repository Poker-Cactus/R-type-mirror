/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** SystemTest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "ecs/Entity.hpp"
#include "ecs/World.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Transform.hpp"
#include "ecs/components/Velocity.hpp"
#include "ecs/systems/MovementSystem.hpp"

// ============================================================================
// MOVEMENT SYSTEM TESTS
// ============================================================================

TEST_SUITE("MovementSystem")
{
  TEST_CASE("MovementSystem - System Registration")
  {
    ecs::World world;

    SUBCASE("Can register MovementSystem")
    {
      CHECK_NOTHROW(world.registerSystem<ecs::MovementSystem>());
    }
  }

  TEST_CASE("MovementSystem - Player Movement Demo")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    // Créer un joueur comme dans le main
    Entity player = world.createEntity();
    world.addComponent(player, Transform{100.0f, 200.0f, 0.0f});
    world.addComponent(player, Velocity{5.0f, 0.0f});
    world.addComponent(player, Health{100, 100});

    SUBCASE("Player initial position")
    {
      auto &transform = world.getComponent<Transform>(player);
      CHECK(transform.x == 100.0f);
      CHECK(transform.y == 200.0f);
      CHECK(transform.rotation == 0.0f);
    }

    SUBCASE("Player moves right after one update")
    {
      constexpr float deltaTime = 0.016f; // 60 FPS
      world.update(deltaTime);

      auto &transform = world.getComponent<Transform>(player);
      CHECK(transform.x == doctest::Approx(100.0f + 5.0f * deltaTime));
      CHECK(transform.y == 200.0f); // Y unchanged
    }

    SUBCASE("Player accumulates movement over multiple frames")
    {
      constexpr float deltaTime = 0.016f;
      constexpr int frames = 5;

      for (int i = 0; i < frames; ++i) {
        world.update(deltaTime);
      }

      auto &transform = world.getComponent<Transform>(player);
      float expectedX = 100.0f + (5.0f * deltaTime * frames);
      CHECK(transform.x == doctest::Approx(expectedX));
      CHECK(transform.y == 200.0f);
    }

    SUBCASE("Player health is not affected by movement")
    {
      world.update(0.016f);

      auto &health = world.getComponent<Health>(player);
      CHECK(health.hp == 100);
      CHECK(health.maxHp == 100);
    }
  }

  TEST_CASE("MovementSystem - Enemy Movement Demo")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    // Créer un ennemi comme dans le main
    Entity enemy = world.createEntity();
    world.addComponent(enemy, Transform{500.0f, 300.0f, 0.0f});
    world.addComponent(enemy, Velocity{-2.0f, 0.0f});

    SUBCASE("Enemy initial position")
    {
      auto &transform = world.getComponent<Transform>(enemy);
      CHECK(transform.x == 500.0f);
      CHECK(transform.y == 300.0f);
    }

    SUBCASE("Enemy moves left (negative velocity)")
    {
      constexpr float deltaTime = 0.016f;
      world.update(deltaTime);

      auto &transform = world.getComponent<Transform>(enemy);
      CHECK(transform.x == doctest::Approx(500.0f + (-2.0f * deltaTime)));
      CHECK(transform.y == 300.0f);
    }

    SUBCASE("Enemy moves over 5 frames")
    {
      constexpr float deltaTime = 0.016f;
      constexpr int frames = 5;

      for (int i = 0; i < frames; ++i) {
        world.update(deltaTime);
      }

      auto &transform = world.getComponent<Transform>(enemy);
      float expectedX = 500.0f + (-2.0f * deltaTime * frames);
      CHECK(transform.x == doctest::Approx(expectedX));
      CHECK(transform.y == 300.0f);
    }
  }

  TEST_CASE("MovementSystem - Background Without Velocity")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    // Créer un background sans Velocity
    Entity background = world.createEntity();
    world.addComponent(background, Transform{0.0f, 0.0f, 0.0f, 1.0f});

    SUBCASE("Background does not move (no Velocity component)")
    {
      constexpr float deltaTime = 0.016f;

      for (int i = 0; i < 10; ++i) {
        world.update(deltaTime);
      }

      auto &transform = world.getComponent<Transform>(background);
      CHECK(transform.x == 0.0f);
      CHECK(transform.y == 0.0f);
      CHECK(transform.rotation == 0.0f);
    }
  }

  TEST_CASE("MovementSystem - Multiple Entities Demo Scenario")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    // Recréer le scénario complet du main
    Entity player = world.createEntity();
    world.addComponent(player, Transform{100.0f, 200.0f, 0.0f});
    world.addComponent(player, Velocity{5.0f, 0.0f});
    world.addComponent(player, Health{100, 100});

    Entity enemy = world.createEntity();
    world.addComponent(enemy, Transform{500.0f, 300.0f, 0.0f});
    world.addComponent(enemy, Velocity{-2.0f, 0.0f});

    Entity background = world.createEntity();
    world.addComponent(background, Transform{0.0f, 0.0f, 0.0f});

    SUBCASE("Only entities with Velocity move")
    {
      constexpr float deltaTime = 0.016f;
      world.update(deltaTime);

      auto &playerTransform = world.getComponent<Transform>(player);
      auto &enemyTransform = world.getComponent<Transform>(enemy);
      auto &bgTransform = world.getComponent<Transform>(background);

      // Player et Enemy bougent
      CHECK(playerTransform.x > 100.0f);
      CHECK(enemyTransform.x < 500.0f);

      // Background ne bouge pas
      CHECK(bgTransform.x == 0.0f);
      CHECK(bgTransform.y == 0.0f);
    }

    SUBCASE("Simulate 5 frames like in main demo")
    {
      constexpr int maxFrames = 5;
      constexpr float deltaTime = 0.016f;

      float initialPlayerX = 100.0f;
      float initialEnemyX = 500.0f;

      for (int frame = 0; frame < maxFrames; ++frame) {
        world.update(deltaTime);
      }

      auto &playerTransform = world.getComponent<Transform>(player);
      auto &enemyTransform = world.getComponent<Transform>(enemy);

      float expectedPlayerX = initialPlayerX + (5.0f * deltaTime * maxFrames);
      float expectedEnemyX = initialEnemyX + (-2.0f * deltaTime * maxFrames);

      CHECK(playerTransform.x == doctest::Approx(expectedPlayerX));
      CHECK(playerTransform.y == 200.0f);

      CHECK(enemyTransform.x == doctest::Approx(expectedEnemyX));
      CHECK(enemyTransform.y == 300.0f);
    }
  }

  TEST_CASE("MovementSystem - 60 FPS Simulation")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    Entity entity = world.createEntity();
    world.addComponent(entity, Transform{0.0f, 0.0f, 0.0f});
    world.addComponent(entity, Velocity{100.0f, 50.0f});

    SUBCASE("Movement at 60 FPS over 1 second")
    {
      constexpr float deltaTime = 0.016f; // ~60 FPS
      constexpr int framesPerSecond = 60;

      for (int i = 0; i < framesPerSecond; ++i) {
        world.update(deltaTime);
      }

      auto &transform = world.getComponent<Transform>(entity);

      // Après ~1 seconde à 60 FPS
      float expectedX = 100.0f * (deltaTime * framesPerSecond);
      float expectedY = 50.0f * (deltaTime * framesPerSecond);

      CHECK(transform.x == doctest::Approx(expectedX).epsilon(0.01));
      CHECK(transform.y == doctest::Approx(expectedY).epsilon(0.01));
    }
  }

  TEST_CASE("MovementSystem - Diagonal Movement")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    Entity entity = world.createEntity();
    world.addComponent(entity, Transform{0.0f, 0.0f, 0.0f});
    world.addComponent(entity, Velocity{3.0f, 4.0f}); // Diagonal

    SUBCASE("Entity moves diagonally")
    {
      constexpr float deltaTime = 1.0f;
      world.update(deltaTime);

      auto &transform = world.getComponent<Transform>(entity);
      CHECK(transform.x == 3.0f);
      CHECK(transform.y == 4.0f);
    }

    SUBCASE("Rotation unchanged during diagonal movement")
    {
      world.update(1.0f);
      auto &transform = world.getComponent<Transform>(entity);
      CHECK(transform.rotation == 0.0f);
    }
  }

  TEST_CASE("MovementSystem - Zero Velocity")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    Entity entity = world.createEntity();
    world.addComponent(entity, Transform{50.0f, 100.0f, 45.0f});
    world.addComponent(entity, Velocity{0.0f, 0.0f});

    SUBCASE("Entity with zero velocity stays still")
    {
      constexpr float deltaTime = 0.016f;

      for (int i = 0; i < 100; ++i) {
        world.update(deltaTime);
      }

      auto &transform = world.getComponent<Transform>(entity);
      CHECK(transform.x == 50.0f);
      CHECK(transform.y == 100.0f);
      CHECK(transform.rotation == 45.0f);
    }
  }

  TEST_CASE("MovementSystem - Dynamic Velocity Change")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    Entity entity = world.createEntity();
    world.addComponent(entity, Transform{0.0f, 0.0f, 0.0f});
    world.addComponent(entity, Velocity{10.0f, 0.0f});

    SUBCASE("Change velocity during runtime")
    {
      // Premier mouvement
      world.update(1.0f);
      auto &transform = world.getComponent<Transform>(entity);
      CHECK(transform.x == 10.0f);

      // Changer la vélocité (comme si le joueur change de direction)
      auto &velocity = world.getComponent<Velocity>(entity);
      velocity.dx = 0.0f;
      velocity.dy = 15.0f;

      // Deuxième mouvement
      world.update(1.0f);
      CHECK(transform.x == 10.0f); // X inchangé
      CHECK(transform.y == 15.0f); // Y augmenté
    }

    SUBCASE("Stop and restart movement")
    {
      world.update(1.0f);
      auto &transform = world.getComponent<Transform>(entity);
      CHECK(transform.x == 10.0f);

      // Arrêter
      auto &velocity = world.getComponent<Velocity>(entity);
      velocity.dx = 0.0f;
      velocity.dy = 0.0f;

      world.update(1.0f);
      CHECK(transform.x == 10.0f); // Pas bougé

      // Redémarrer
      velocity.dx = 5.0f;
      world.update(1.0f);
      CHECK(transform.x == 15.0f);
    }
  }

  TEST_CASE("MovementSystem - Large Scale Movement")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    Entity entity = world.createEntity();
    world.addComponent(entity, Transform{0.0f, 0.0f, 0.0f});
    world.addComponent(entity, Velocity{1000.0f, 2000.0f});

    SUBCASE("Handle large velocity values")
    {
      world.update(0.1f);
      auto &transform = world.getComponent<Transform>(entity);
      CHECK(transform.x == 100.0f);
      CHECK(transform.y == 200.0f);
    }
  }

  TEST_CASE("MovementSystem - Precision Test")
  {
    ecs::World world;
    world.registerSystem<ecs::MovementSystem>();

    Entity entity = world.createEntity();
    world.addComponent(entity, Transform{0.0f, 0.0f, 0.0f});
    world.addComponent(entity, Velocity{0.1f, 0.1f});

    SUBCASE("Small incremental movements maintain precision")
    {
      constexpr float deltaTime = 0.016f;
      constexpr int iterations = 1000;

      for (int i = 0; i < iterations; ++i) {
        world.update(deltaTime);
      }

      auto &transform = world.getComponent<Transform>(entity);
      float expected = 0.1f * deltaTime * iterations;

      CHECK(transform.x == doctest::Approx(expected).epsilon(0.001));
      CHECK(transform.y == doctest::Approx(expected).epsilon(0.001));
    }
  }
}
