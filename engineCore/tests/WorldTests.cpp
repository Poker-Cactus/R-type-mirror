/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** World Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include "ecs/ISystem.hpp"
#include "ecs/World.hpp"
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
};

// ============================================================================
// TEST SYSTEMS
// ============================================================================

class TestSystem : public ecs::ISystem
{
private:
  int updateCount = 0;

public:
  void update(ecs::World &world, float deltaTime) override
  {
    updateCount++;
    (void)world;
    (void)deltaTime;
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

  [[nodiscard]] int getUpdateCount() const { return updateCount; }
};

class MovementSystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    (void)deltaTime;
    // Would iterate entities with Position and Velocity
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<Position>());
    sig.set(ecs::getComponentId<Velocity>());
    return sig;
  }
};

// ============================================================================
// WORLD TESTS
// ============================================================================

TEST_SUITE("World")
{
  TEST_CASE("System management")
  {
    ecs::World world;

    SUBCASE("Register system")
    {
      auto &system = world.registerSystem<TestSystem>();
      CHECK(world.hasSystem<TestSystem>());
      CHECK(system.getUpdateCount() == 0);
    }

    SUBCASE("Get system")
    {
      world.registerSystem<TestSystem>();

      auto *system = world.getSystem<TestSystem>();
      CHECK(system != nullptr);
    }

    SUBCASE("Get non-existent system returns nullptr")
    {
      auto *system = world.getSystem<TestSystem>();
      CHECK(system == nullptr);
    }

    SUBCASE("Remove system")
    {
      world.registerSystem<TestSystem>();
      CHECK(world.hasSystem<TestSystem>());

      world.removeSystem<TestSystem>();
      CHECK_FALSE(world.hasSystem<TestSystem>());
    }

    SUBCASE("System count")
    {
      CHECK(world.getSystemCount() == 0);

      world.registerSystem<TestSystem>();
      CHECK(world.getSystemCount() == 1);

      world.registerSystem<MovementSystem>();
      CHECK(world.getSystemCount() == 2);
    }

    SUBCASE("Clear systems")
    {
      world.registerSystem<TestSystem>();
      world.registerSystem<MovementSystem>();

      world.clearSystems();
      CHECK(world.getSystemCount() == 0);
    }
  }

  TEST_CASE("Update systems")
  {
    ecs::World world;

    SUBCASE("Update calls all systems")
    {
      auto &sys1 = world.registerSystem<TestSystem>();

      world.update(0.016F);
      CHECK(sys1.getUpdateCount() == 1);

      world.update(0.016F);
      CHECK(sys1.getUpdateCount() == 2);
    }
  }

  TEST_CASE("Component management")
  {
    ecs::World world;
    Entity entity = 0;

    SUBCASE("Add component")
    {
      world.addComponent(entity, Position{.x = 1.0F, .y = 2.0F});

      CHECK(world.hasComponent<Position>(entity));
    }

    SUBCASE("Get component")
    {
      world.addComponent(entity, Position{.x = 10.0F, .y = 20.0F});

      auto &pos = world.getComponent<Position>(entity);
      CHECK(pos.x == 10.0F);
      CHECK(pos.y == 20.0F);
    }

    SUBCASE("Modify component")
    {
      world.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});

      auto &pos = world.getComponent<Position>(entity);
      pos.x = 5.0F;
      pos.y = 10.0F;

      CHECK(world.getComponent<Position>(entity).x == 5.0F);
      CHECK(world.getComponent<Position>(entity).y == 10.0F);
    }

    SUBCASE("Has component")
    {
      CHECK_FALSE(world.hasComponent<Position>(entity));

      world.addComponent(entity, Position{.x = 0.0F, .y = 0.0F});

      CHECK(world.hasComponent<Position>(entity));
    }

    SUBCASE("Remove component")
    {
      world.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      CHECK(world.hasComponent<Position>(entity));

      world.removeComponent<Position>(entity);
      CHECK_FALSE(world.hasComponent<Position>(entity));
    }

    SUBCASE("Multiple components per entity")
    {
      world.addComponent(entity, Position{.x = 1.0F, .y = 2.0F});
      world.addComponent(entity, Velocity{.dx = 3.0F, .dy = 4.0F});
      world.addComponent(entity, Health{100});

      CHECK(world.hasComponent<Position>(entity));
      CHECK(world.hasComponent<Velocity>(entity));
      CHECK(world.hasComponent<Health>(entity));
    }

    SUBCASE("Remove all components")
    {
      world.addComponent(entity, Position{.x = 1.0F, .y = 1.0F});
      world.addComponent(entity, Velocity{.dx = 1.0F, .dy = 1.0F});
      world.addComponent(entity, Health{100});

      world.removeAllComponents(entity);

      CHECK_FALSE(world.hasComponent<Position>(entity));
      CHECK_FALSE(world.hasComponent<Velocity>(entity));
      CHECK_FALSE(world.hasComponent<Health>(entity));
    }
  }

  TEST_CASE("Entity signature")
  {
    ecs::World world;
    Entity entity = 0;

    SUBCASE("Get entity signature")
    {
      world.addComponent(entity, Position{.x = 0.0F, .y = 0.0F});

      const auto &sig = world.getEntitySignature(entity);
      CHECK(sig.test(ecs::getComponentId<Position>()));
    }

    SUBCASE("Signature tracks multiple components")
    {
      world.addComponent(entity, Position{.x = 0.0F, .y = 0.0F});
      world.addComponent(entity, Velocity{.dx = 0.0F, .dy = 0.0F});

      const auto &sig = world.getEntitySignature(entity);
      CHECK(sig.test(ecs::getComponentId<Position>()));
      CHECK(sig.test(ecs::getComponentId<Velocity>()));
      CHECK_FALSE(sig.test(ecs::getComponentId<Health>()));
    }
  }

  TEST_CASE("Const correctness")
  {
    ecs::World world;
    Entity entity = 0;
    world.addComponent(entity, Position{.x = 1.0F, .y = 2.0F});

    SUBCASE("Const getComponent")
    {
      const ecs::World &constWorld = world;
      const auto &pos = constWorld.getComponent<Position>(entity);
      CHECK(pos.x == 1.0F);
      CHECK(pos.y == 2.0F);
    }

    SUBCASE("Const hasComponent")
    {
      const ecs::World &constWorld = world;
      CHECK(constWorld.hasComponent<Position>(entity));
    }

    SUBCASE("Const getSystem")
    {
      world.registerSystem<TestSystem>();
      const ecs::World &constWorld = world;
      const auto *system = constWorld.getSystem<TestSystem>();
      CHECK(system != nullptr);
    }

    SUBCASE("Const getSystemCount")
    {
      world.registerSystem<TestSystem>();
      const ecs::World &constWorld = world;
      CHECK(constWorld.getSystemCount() == 1);
    }
  }

  TEST_CASE("Integration test: Simple game loop simulation")
  {
    ecs::World world;

    SUBCASE("Create entities with components and systems")
    {
      // Register systems
      world.registerSystem<MovementSystem>();

      // Create entities
      Entity player = 0;
      Entity enemy = 1;

      // Add components
      world.addComponent(player, Position{.x = 0.0F, .y = 0.0F});
      world.addComponent(player, Velocity{.dx = 1.0F, .dy = 0.0F});
      world.addComponent(player, Health{100});

      world.addComponent(enemy, Position{.x = 10.0F, .y = 10.0F});
      world.addComponent(enemy, Velocity{.dx = -1.0F, .dy = 0.0F});
      world.addComponent(enemy, Health{50});

      // Verify setup
      CHECK(world.hasComponent<Position>(player));
      CHECK(world.hasComponent<Velocity>(player));
      CHECK(world.hasComponent<Health>(player));

      CHECK(world.hasComponent<Position>(enemy));
      CHECK(world.hasComponent<Velocity>(enemy));
      CHECK(world.hasComponent<Health>(enemy));

      // Simulate update
      CHECK_NOTHROW(world.update(0.016F));

      // Modify components
      auto &playerPos = world.getComponent<Position>(player);
      const auto &playerVel = world.getComponent<Velocity>(player);
      playerPos.x += playerVel.dx * 0.016F;

      CHECK(playerPos.x > 0.0F);
    }
  }

  TEST_CASE("Multiple entities management")
  {
    ecs::World world;

    SUBCASE("Independent component management per entity")
    {
      Entity ent0 = 0;
      Entity ent1 = 1;
      Entity ent2 = 2;

      world.addComponent(ent0, Position{.x = 1.0F, .y = 1.0F});
      world.addComponent(ent1, Position{.x = 2.0F, .y = 2.0F});
      world.addComponent(ent2, Position{.x = 3.0F, .y = 3.0F});

      CHECK(world.getComponent<Position>(ent0).x == 1.0F);
      CHECK(world.getComponent<Position>(ent1).x == 2.0F);
      CHECK(world.getComponent<Position>(ent2).x == 3.0F);
    }

    SUBCASE("Different components per entity")
    {
      Entity ent0 = 0;
      Entity ent1 = 1;

      world.addComponent(ent0, Position{.x = 1.0F, .y = 1.0F});
      world.addComponent(ent0, Velocity{.dx = 2.0F, .dy = 2.0F});

      world.addComponent(ent1, Position{.x = 3.0F, .y = 3.0F});
      world.addComponent(ent1, Health{50});

      CHECK(world.hasComponent<Position>(ent0));
      CHECK(world.hasComponent<Velocity>(ent0));
      CHECK_FALSE(world.hasComponent<Health>(ent0));

      CHECK(world.hasComponent<Position>(ent1));
      CHECK_FALSE(world.hasComponent<Velocity>(ent1));
      CHECK(world.hasComponent<Health>(ent1));
    }
  }

  TEST_CASE("Error handling")
  {
    ecs::World world;

    SUBCASE("Get non-existent component throws")
    {
      Entity entity = 0;
      CHECK_THROWS_AS(world.getComponent<Position>(entity), std::out_of_range);
    }

    SUBCASE("Remove non-existent component doesn't crash")
    {
      Entity entity = 0;
      CHECK_NOTHROW(world.removeComponent<Position>(entity));
    }
  }
}
