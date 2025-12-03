/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ComponentStorage Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/ComponentStorage.hpp"
#include "ecs/Entity.hpp"
#include <doctest/doctest.h>
#include <string>

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct Position {
  float x;
  float y;

  bool operator==(const Position &other) const { return x == other.x && y == other.y; }
};

struct Velocity {
  float dx;
  float dy;
};

struct Health {
  int hp;
  int maxHp;
};

// ============================================================================
// COMPONENT STORAGE TESTS
// ============================================================================

TEST_SUITE("ComponentStorage")
{
  TEST_CASE("Add and retrieve components")
  {
    ComponentStorage<Position> storage;

    SUBCASE("Add single component")
    {
      Entity entity = 0;
      Position pos{10.0f, 20.0f};

      storage.addComponent(entity, pos);

      CHECK(storage.hasComponent(entity));
      auto &retrieved = storage.getComponent(entity);
      CHECK(retrieved.x == 10.0f);
      CHECK(retrieved.y == 20.0f);
    }

    SUBCASE("Add multiple components")
    {
      storage.addComponent(0, Position{1.0f, 2.0f});
      storage.addComponent(1, Position{3.0f, 4.0f});
      storage.addComponent(2, Position{5.0f, 6.0f});

      CHECK(storage.hasComponent(0));
      CHECK(storage.hasComponent(1));
      CHECK(storage.hasComponent(2));

      CHECK(storage.getComponent(0) == Position{1.0f, 2.0f});
      CHECK(storage.getComponent(1) == Position{3.0f, 4.0f});
      CHECK(storage.getComponent(2) == Position{5.0f, 6.0f});
    }

    SUBCASE("Add component to non-sequential entities")
    {
      storage.addComponent(0, Position{1.0f, 1.0f});
      storage.addComponent(5, Position{5.0f, 5.0f});
      storage.addComponent(10, Position{10.0f, 10.0f});

      CHECK(storage.hasComponent(0));
      CHECK_FALSE(storage.hasComponent(3));
      CHECK(storage.hasComponent(5));
      CHECK(storage.hasComponent(10));
    }
  }

  TEST_CASE("Update existing components")
  {
    ComponentStorage<Position> storage;

    SUBCASE("Update component by adding again")
    {
      Entity entity = 0;
      storage.addComponent(entity, Position{1.0f, 1.0f});

      CHECK(storage.getComponent(entity) == Position{1.0f, 1.0f});

      // Update by adding again (covers lines 28-29)
      storage.addComponent(entity, Position{2.0f, 2.0f});

      CHECK(storage.getComponent(entity) == Position{2.0f, 2.0f});
    }

    SUBCASE("Update component by reference")
    {
      Entity entity = 0;
      storage.addComponent(entity, Position{1.0f, 1.0f});

      auto &pos = storage.getComponent(entity);
      pos.x = 100.0f;
      pos.y = 200.0f;

      CHECK(storage.getComponent(entity).x == 100.0f);
      CHECK(storage.getComponent(entity).y == 200.0f);
    }
  }

  TEST_CASE("Remove components")
  {
    ComponentStorage<Position> storage;

    SUBCASE("Remove existing component")
    {
      Entity entity = 0;
      storage.addComponent(entity, Position{1.0f, 1.0f});

      CHECK(storage.hasComponent(entity));

      storage.removeComponent(entity);

      CHECK_FALSE(storage.hasComponent(entity));
    }

    SUBCASE("Remove non-existent component (no crash)")
    {
      Entity entity = 999;
      CHECK_NOTHROW(storage.removeComponent(entity));
      CHECK_FALSE(storage.hasComponent(entity));
    }

    SUBCASE("Remove and re-add component")
    {
      Entity entity = 0;
      storage.addComponent(entity, Position{1.0f, 1.0f});
      storage.removeComponent(entity);

      CHECK_FALSE(storage.hasComponent(entity));

      storage.addComponent(entity, Position{2.0f, 2.0f});
      CHECK(storage.hasComponent(entity));
      CHECK(storage.getComponent(entity) == Position{2.0f, 2.0f});
    }

    SUBCASE("Remove from multiple entities")
    {
      storage.addComponent(0, Position{1.0f, 1.0f});
      storage.addComponent(1, Position{2.0f, 2.0f});
      storage.addComponent(2, Position{3.0f, 3.0f});

      // Remove middle entity (covers swap lines 52-54)
      storage.removeComponent(1);

      CHECK(storage.hasComponent(0));
      CHECK_FALSE(storage.hasComponent(1));
      CHECK(storage.hasComponent(2));
    }

    SUBCASE("Test hasComponent with boundary cases")
    {
      // Test hasComponent on entity without component (covers line 64)
      CHECK_FALSE(storage.hasComponent(999));

      // Add component and verify
      storage.addComponent(0, Position{1.0f, 1.0f});
      CHECK(storage.hasComponent(0));
    }
  }

  TEST_CASE("Error handling")
  {
    ComponentStorage<Position> storage;

    SUBCASE("Get component from entity without component throws")
    {
      Entity entity = 0;
      CHECK_THROWS_AS(storage.getComponent(entity), std::out_of_range);
    }

    SUBCASE("Get component from non-existent entity throws")
    {
      CHECK_THROWS_AS(storage.getComponent(999), std::out_of_range);
    }

    SUBCASE("Const version also throws on missing component")
    {
      const ComponentStorage<Position> &constStorage = storage;
      CHECK_THROWS_AS(constStorage.getComponent(0), std::out_of_range);
    }
  }

  TEST_CASE("Different component types")
  {
    SUBCASE("Velocity components")
    {
      ComponentStorage<Velocity> storage;
      Entity entity = 0;

      storage.addComponent(entity, Velocity{5.0f, -3.0f});

      CHECK(storage.hasComponent(entity));
      auto &vel = storage.getComponent(entity);
      CHECK(vel.dx == 5.0f);
      CHECK(vel.dy == -3.0f);
    }

    SUBCASE("Health components")
    {
      ComponentStorage<Health> storage;
      Entity entity = 0;

      storage.addComponent(entity, Health{100, 100});

      CHECK(storage.hasComponent(entity));
      auto &health = storage.getComponent(entity);
      CHECK(health.hp == 100);
      CHECK(health.maxHp == 100);
    }
  }

  TEST_CASE("Sparse array efficiency")
  {
    ComponentStorage<Position> storage;

    SUBCASE("Add components with large entity IDs")
    {
      storage.addComponent(1000, Position{1.0f, 1.0f});
      storage.addComponent(2000, Position{2.0f, 2.0f});

      CHECK(storage.hasComponent(1000));
      CHECK(storage.hasComponent(2000));
      CHECK_FALSE(storage.hasComponent(1500));
    }

    SUBCASE("Remove middle entity maintains other entities")
    {
      storage.addComponent(0, Position{0.0f, 0.0f});
      storage.addComponent(1, Position{1.0f, 1.0f});
      storage.addComponent(2, Position{2.0f, 2.0f});
      storage.addComponent(3, Position{3.0f, 3.0f});

      storage.removeComponent(1);

      CHECK(storage.hasComponent(0));
      CHECK_FALSE(storage.hasComponent(1));
      CHECK(storage.hasComponent(2));
      CHECK(storage.hasComponent(3));

      // Verify values are still correct
      CHECK(storage.getComponent(0).x == 0.0f);
      CHECK(storage.getComponent(2).x == 2.0f);
      CHECK(storage.getComponent(3).x == 3.0f);
    }
  }
}
