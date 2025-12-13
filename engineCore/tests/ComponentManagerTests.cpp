/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ComponentManager Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/ComponentManager.hpp"
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include <doctest/doctest.h>
#include <stdexcept>
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

  bool operator==(const Velocity &other) const { return dx == other.dx && dy == other.dy; }
};

struct Health {
  int hp;
  int maxHp;
};

struct Name {
  std::string value;
};

// ============================================================================
// COMPONENT MANAGER TESTS
// ============================================================================

TEST_SUITE("ComponentManager")
{
  TEST_CASE("Add and retrieve components")
  {
    ComponentManager manager;
    Entity entity = 0;

    SUBCASE("Add single component")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 2.0F});

      CHECK(manager.hasComponent<Position>(entity));
      auto &pos = manager.getComponent<Position>(entity);
      CHECK(pos.x == 1.0F);
      CHECK(pos.y == 2.0F);
    }

    SUBCASE("Add multiple component types to same entity")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 2.0F});
      manager.addComponent(entity, Velocity{.dx = 3.0F, .dy = 4.0F});
      manager.addComponent(entity, Health{.hp = 100, .maxHp = 100});

      CHECK(manager.hasComponent<Position>(entity));
      CHECK(manager.hasComponent<Velocity>(entity));
      CHECK(manager.hasComponent<Health>(entity));
    }

    SUBCASE("Add same component type to multiple entities")
    {
      Entity ent0 = 0;
      Entity ent1 = 1;
      Entity ent2 = 2;

      manager.addComponent(ent0, Position{.x = 1.0F, .y = 1.0F});
      manager.addComponent(ent1, Position{.x = 2.0F, .y = 2.0F});
      manager.addComponent(ent2, Position{.x = 3.0F, .y = 3.0F});

      CHECK(manager.getComponent<Position>(ent0).x == 1.0F);
      CHECK(manager.getComponent<Position>(ent1).x == 2.0F);
      CHECK(manager.getComponent<Position>(ent2).x == 3.0F);
    }
  }

  // Component signature tracking tests removed
  // EntityManager is now the single source of truth for signatures
  // See WorldTests.cpp for signature-related tests

  TEST_CASE("Remove components")
  {
    ComponentManager manager;
    Entity entity = 0;

    SUBCASE("Remove existing component")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      CHECK(manager.hasComponent<Position>(entity));

      manager.removeComponent<Position>(entity);

      CHECK_FALSE(manager.hasComponent<Position>(entity));
    }

    // Signature tests removed - EntityManager now handles signatures

    SUBCASE("Remove non-existent component (no crash)")
    {
      CHECK_NOTHROW(manager.removeComponent<Position>(entity));
    }

    SUBCASE("Remove one component doesn't affect others")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      manager.addComponent(entity, Velocity{.dx = 2.0F, .dy = 2.0F});
      manager.addComponent(entity, Health{.hp = 100, .maxHp = 100});

      manager.removeComponent<Velocity>(entity);

      CHECK(manager.hasComponent<Position>(entity));
      CHECK_FALSE(manager.hasComponent<Velocity>(entity));
      CHECK(manager.hasComponent<Health>(entity));
    }
  }

  TEST_CASE("Remove all components")
  {
    ComponentManager manager;
    Entity entity = 0;

    SUBCASE("Remove all components from entity")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      manager.addComponent(entity, Velocity{.dx = 2.0F, .dy = 2.0F});
      manager.addComponent(entity, Health{.hp = 100, .maxHp = 100});

      manager.removeAllComponents(entity);

      CHECK_FALSE(manager.hasComponent<Position>(entity));
      CHECK_FALSE(manager.hasComponent<Velocity>(entity));
      CHECK_FALSE(manager.hasComponent<Health>(entity));
    }

    // Signature tests removed - EntityManager now handles signatures

    SUBCASE("Remove all on entity without components (no crash)")
    {
      CHECK_NOTHROW(manager.removeAllComponents(999));
    }
  }

  TEST_CASE("Modify components")
  {
    ComponentManager manager;
    Entity entity = 0;

    SUBCASE("Modify component through reference")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});

      auto &pos = manager.getComponent<Position>(entity);
      pos.x = 10.0F;
      pos.y = 20.0F;

      CHECK(manager.getComponent<Position>(entity).x == 10.0F);
      CHECK(manager.getComponent<Position>(entity).y == 20.0F);
    }

    SUBCASE("Update component by adding again")
    {
      manager.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      manager.addComponent(entity, Position{.x = 5.0F, .y = 5.0F});

      CHECK(manager.getComponent<Position>(entity) == Position{5.0F, 5.0F});
    }
  }

  TEST_CASE("Const correctness")
  {
    ComponentManager manager;
    Entity entity = 0;
    manager.addComponent(entity, Position{.x = 1.0F, .y = 2.0F});

    SUBCASE("Const getComponent")
    {
      const ComponentManager &constManager = manager;
      const auto &pos = constManager.getComponent<Position>(entity);
      CHECK(pos.x == 1.0F);
      CHECK(pos.y == 2.0F);
    }

    SUBCASE("Const hasComponent")
    {
      const ComponentManager &constManager = manager;
      CHECK(constManager.hasComponent<Position>(entity));
      CHECK_FALSE(constManager.hasComponent<Velocity>(entity));
    }

    // Signature tests removed - EntityManager now handles signatures
  }

  TEST_CASE("Error handling")
  {
    ComponentManager manager;
    Entity entity = 0;

    SUBCASE("Get non-existent component throws")
    {
      CHECK_THROWS_AS(manager.getComponent<Position>(entity), std::out_of_range);
    }

    SUBCASE("Const version also throws")
    {
      const ComponentManager &constManager = manager;
      CHECK_THROWS_AS(static_cast<void>(constManager.getComponent<Position>(entity)), std::out_of_range);
    }
  }

  TEST_CASE("Multiple entity management")
  {
    ComponentManager manager;

    SUBCASE("Independent component management per entity")
    {
      [[maybe_unused]] Entity ent0 = 0;
      [[maybe_unused]] Entity ent1 = 1;

      manager.addComponent(ent0, Position{.x = 1.0F, .y = 1.0F});
      manager.addComponent(ent0, Velocity{.dx = 2.0F, .dy = 2.0F});

      manager.addComponent(ent1, Position{.x = 3.0F, .y = 3.0F});
      manager.addComponent(ent1, Health{.hp = 50, .maxHp = 100});

      // Entity 0 checks
      CHECK(manager.hasComponent<Position>(ent0));
      CHECK(manager.hasComponent<Velocity>(ent0));
      CHECK_FALSE(manager.hasComponent<Health>(ent0));

      // Entity 1 checks
      CHECK(manager.hasComponent<Position>(ent1));
      CHECK_FALSE(manager.hasComponent<Velocity>(ent1));
      CHECK(manager.hasComponent<Health>(ent1));
    }

    SUBCASE("Removing from one entity doesn't affect others")
    {
      Entity ent0 = 0;
      Entity ent1 = 1;

      manager.addComponent(ent0, Position{.x = 1.0F, .y = 1.0F});
      manager.addComponent(ent1, Position{.x = 2.0F, .y = 2.0F});

      manager.removeComponent<Position>(ent0);

      CHECK_FALSE(manager.hasComponent<Position>(ent0));
      CHECK(manager.hasComponent<Position>(ent1));
    }
  }

  TEST_CASE("Complex component types")
  {
    ComponentManager manager;
    Entity entity = 0;

    SUBCASE("String components")
    {
      manager.addComponent(entity, Name{"TestEntity"});

      CHECK(manager.hasComponent<Name>(entity));
      CHECK(manager.getComponent<Name>(entity).value == "TestEntity");
    }

    SUBCASE("Modify string component")
    {
      manager.addComponent(entity, Name{"Original"});

      auto &name = manager.getComponent<Name>(entity);
      name.value = "Modified";

      CHECK(manager.getComponent<Name>(entity).value == "Modified");
    }
  }

  TEST_CASE("Component ID consistency")
  {
    SUBCASE("Same component type gets same ID")
    {
      auto id1 = ecs::getComponentId<Position>();
      auto id2 = ecs::getComponentId<Position>();

      CHECK(id1 == id2);
    }

    SUBCASE("Different component types get different IDs")
    {
      auto posId = ecs::getComponentId<Position>();
      auto velId = ecs::getComponentId<Velocity>();
      auto healthId = ecs::getComponentId<Health>();

      CHECK(posId != velId);
      CHECK(velId != healthId);
      CHECK(posId != healthId);
    }
  }
}
