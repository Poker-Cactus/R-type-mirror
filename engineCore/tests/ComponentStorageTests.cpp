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
#include <stdexcept>

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
      Position pos{.x = 10.0F, .y = 20.0F};

      storage.addComponent(entity, pos);

      CHECK(storage.hasComponent(entity));
      auto &retrieved = storage.getComponent(entity);
      CHECK(retrieved.x == 10.0F);
      CHECK(retrieved.y == 20.0F);
    }

    SUBCASE("Add multiple components")
    {
      storage.addComponent(0, Position{.x = 1.0F, .y = 2.0F});
      storage.addComponent(1, Position{.x = 3.0F, .y = 4.0F});
      storage.addComponent(2, Position{.x = 5.0F, .y = 6.0F});

      CHECK(storage.hasComponent(0));
      CHECK(storage.hasComponent(1));
      CHECK(storage.hasComponent(2));

      CHECK(storage.getComponent(0) == Position{1.0F, 2.0F});
      CHECK(storage.getComponent(1) == Position{3.0F, 4.0F});
      CHECK(storage.getComponent(2) == Position{5.0F, 6.0F});
    }

    SUBCASE("Add component to non-sequential entities")
    {
      storage.addComponent(0, Position{.x = 1.0F, .y = 1.0F});
      storage.addComponent(5, Position{.x = 5.0F, .y = 5.0F});
      storage.addComponent(10, Position{.x = 10.0F, .y = 10.0F});

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
      storage.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});

      CHECK(storage.getComponent(entity) == Position{1.0F, 1.0F});

      // Update by adding again (covers lines 28-29)
      storage.addComponent(entity, Position{.x = 2.0F, .y = 2.0F});

      CHECK(storage.getComponent(entity) == Position{2.0F, 2.0F});
    }

    SUBCASE("Update component by reference")
    {
      Entity entity = 0;
      storage.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});

      auto &pos = storage.getComponent(entity);
      pos.x = 100.0F;
      pos.y = 200.0F;

      CHECK(storage.getComponent(entity).x == 100.0F);
      CHECK(storage.getComponent(entity).y == 200.0F);
    }
  }

  TEST_CASE("Remove components")
  {
    ComponentStorage<Position> storage;

    SUBCASE("Remove existing component")
    {
      Entity entity = 0;
      storage.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});

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
      storage.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      storage.removeComponent(entity);

      CHECK_FALSE(storage.hasComponent(entity));

      storage.addComponent(entity, Position{.x = 2.0F, .y = 2.0F});
      CHECK(storage.hasComponent(entity));
      CHECK(storage.getComponent(entity) == Position{2.0F, 2.0F});
    }

    SUBCASE("Remove from multiple entities")
    {
      storage.addComponent(0, Position{.x = 1.0F, .y = 1.0F});
      storage.addComponent(1, Position{.x = 2.0F, .y = 2.0F});
      storage.addComponent(2, Position{.x = 3.0F, .y = 3.0F});

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
      storage.addComponent(0, Position{.x = 1.0F, .y = 1.0F});
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
      CHECK_THROWS_AS(static_cast<void>(constStorage.getComponent(0)), std::out_of_range);
    }
  }

  TEST_CASE("Different component types")
  {
    SUBCASE("Velocity components")
    {
      ComponentStorage<Velocity> storage;
      Entity entity = 0;

      storage.addComponent(entity, Velocity{.dx = 5.0F, .dy = -3.0F});

      CHECK(storage.hasComponent(entity));
      auto &vel = storage.getComponent(entity);
      CHECK(vel.dx == 5.0F);
      CHECK(vel.dy == -3.0F);
    }

    SUBCASE("Health components")
    {
      ComponentStorage<Health> storage;
      Entity entity = 0;

      storage.addComponent(entity, Health{.hp = 100, .maxHp = 100});

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
      storage.addComponent(1000, Position{.x = 1.0F, .y = 1.0F});
      storage.addComponent(2000, Position{.x = 2.0F, .y = 2.0F});

      CHECK(storage.hasComponent(1000));
      CHECK(storage.hasComponent(2000));
      CHECK_FALSE(storage.hasComponent(1500));
    }

    SUBCASE("Remove middle entity maintains other entities")
    {
      storage.addComponent(0, Position{.x = 0.0F, .y = 0.0F});
      storage.addComponent(1, Position{.x = 1.0F, .y = 1.0F});
      storage.addComponent(2, Position{.x = 2.0F, .y = 2.0F});
      storage.addComponent(3, Position{.x = 3.0F, .y = 3.0F});

      storage.removeComponent(1);

      CHECK(storage.hasComponent(0));
      CHECK_FALSE(storage.hasComponent(1));
      CHECK(storage.hasComponent(2));
      CHECK(storage.hasComponent(3));

      // Verify values are still correct
      CHECK(storage.getComponent(0).x == 0.0F);
      CHECK(storage.getComponent(2).x == 2.0F);
      CHECK(storage.getComponent(3).x == 3.0F);
    }
  }
}
