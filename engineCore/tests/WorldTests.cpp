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
#include <algorithm>
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
    ecs::Entity entity = world.createEntity();

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
    ecs::Entity entity = world.createEntity();

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
    ecs::Entity entity = world.createEntity();
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
      ecs::Entity player = world.createEntity();
      ecs::Entity enemy = world.createEntity();

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
      ecs::Entity ent0 = world.createEntity();
      ecs::Entity ent1 = world.createEntity();
      ecs::Entity ent2 = world.createEntity();

      world.addComponent(ent0, Position{.x = 1.0F, .y = 1.0F});
      world.addComponent(ent1, Position{.x = 2.0F, .y = 2.0F});
      world.addComponent(ent2, Position{.x = 3.0F, .y = 3.0F});

      CHECK(world.getComponent<Position>(ent0).x == 1.0F);
      CHECK(world.getComponent<Position>(ent1).x == 2.0F);
      CHECK(world.getComponent<Position>(ent2).x == 3.0F);
    }

    SUBCASE("Different components per entity")
    {
      ecs::Entity ent0 = world.createEntity();
      ecs::Entity ent1 = world.createEntity();

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
      ecs::Entity entity = world.createEntity();
      CHECK_THROWS_AS(world.getComponent<Position>(entity), std::out_of_range);
    }

    SUBCASE("Remove non-existent component doesn't crash")
    {
      ecs::Entity entity = world.createEntity();
      CHECK_NOTHROW(world.removeComponent<Position>(entity));
    }
  }

  TEST_CASE("Entity filtering with getEntitiesWithSignature")
  {
    ecs::World world;

    SUBCASE("Entities matching the signature are returned")
    {
      // Create entities with different component combinations
      ecs::Entity player = world.createEntity();
      world.addComponent(player, Position{.x = 1.0F, .y = 2.0F});
      world.addComponent(player, Velocity{.dx = 3.0F, .dy = 4.0F});

      ecs::Entity enemy = world.createEntity();
      world.addComponent(enemy, Position{.x = 5.0F, .y = 6.0F});
      world.addComponent(enemy, Velocity{.dx = 7.0F, .dy = 8.0F});

      ecs::Entity staticObj = world.createEntity();
      world.addComponent(staticObj, Position{.x = 9.0F, .y = 10.0F});

      // Query for entities with Position and Velocity
      ecs::ComponentSignature signature;
      signature.set(ecs::getComponentId<Position>());
      signature.set(ecs::getComponentId<Velocity>());

      std::vector<ecs::Entity> entities;
      world.getEntitiesWithSignature(signature, entities);

      // Both player and enemy should be returned
      CHECK(entities.size() == 2);
      CHECK(std::find(entities.begin(), entities.end(), player) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), enemy) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), staticObj) == entities.end());
    }

    SUBCASE("Entities not matching the signature are excluded")
    {
      ecs::Entity ent1 = world.createEntity();
      world.addComponent(ent1, Position{.x = 1.0F, .y = 1.0F});

      ecs::Entity ent2 = world.createEntity();
      world.addComponent(ent2, Velocity{.dx = 2.0F, .dy = 2.0F});

      ecs::Entity ent3 = world.createEntity();
      world.addComponent(ent3, Health{100});

      // Query for Position + Velocity
      ecs::ComponentSignature signature;
      signature.set(ecs::getComponentId<Position>());
      signature.set(ecs::getComponentId<Velocity>());

      std::vector<ecs::Entity> entities;
      world.getEntitiesWithSignature(signature, entities);

      // No entity has both Position and Velocity
      CHECK(entities.empty());
    }

    SUBCASE("Dead entities are not included in results")
    {
      ecs::Entity alive1 = world.createEntity();
      world.addComponent(alive1, Position{.x = 1.0F, .y = 1.0F});

      ecs::Entity toDestroy = world.createEntity();
      world.addComponent(toDestroy, Position{.x = 2.0F, .y = 2.0F});

      ecs::Entity alive2 = world.createEntity();
      world.addComponent(alive2, Position{.x = 3.0F, .y = 3.0F});

      // Destroy one entity
      world.destroyEntity(toDestroy);

      // Query for Position
      ecs::ComponentSignature signature;
      signature.set(ecs::getComponentId<Position>());

      std::vector<ecs::Entity> entities;
      world.getEntitiesWithSignature(signature, entities);

      // Only alive entities should be returned
      CHECK(entities.size() == 2);
      CHECK(std::find(entities.begin(), entities.end(), alive1) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), alive2) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), toDestroy) == entities.end());
    }

    SUBCASE("The output vector is properly cleared before populating")
    {
      ecs::Entity ent1 = world.createEntity();
      world.addComponent(ent1, Position{.x = 1.0F, .y = 1.0F});

      ecs::Entity ent2 = world.createEntity();
      world.addComponent(ent2, Position{.x = 2.0F, .y = 2.0F});

      ecs::ComponentSignature signature;
      signature.set(ecs::getComponentId<Position>());

      std::vector<ecs::Entity> entities;
      entities.push_back(999); // Pre-populate vector with invalid entity
      entities.push_back(888);

      world.getEntitiesWithSignature(signature, entities);

      // Vector should be cleared and contain only matching entities
      CHECK(entities.size() == 2);
      CHECK(std::find(entities.begin(), entities.end(), 999) == entities.end());
      CHECK(std::find(entities.begin(), entities.end(), 888) == entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent1) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent2) != entities.end());
    }

    SUBCASE("Edge case: empty signature returns all alive entities")
    {
      ecs::Entity ent1 = world.createEntity();
      world.addComponent(ent1, Position{.x = 1.0F, .y = 1.0F});

      ecs::Entity ent2 = world.createEntity();
      world.addComponent(ent2, Velocity{.dx = 2.0F, .dy = 2.0F});

      ecs::Entity ent3 = world.createEntity();
      world.addComponent(ent3, Health{100});

      // Empty signature (no components required)
      ecs::ComponentSignature emptySignature;

      std::vector<ecs::Entity> entities;
      world.getEntitiesWithSignature(emptySignature, entities);

      // All alive entities should match an empty signature
      CHECK(entities.size() == 3);
      CHECK(std::find(entities.begin(), entities.end(), ent1) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent2) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent3) != entities.end());
    }

    SUBCASE("Edge case: no matching entities returns empty vector")
    {
      ecs::Entity ent1 = world.createEntity();
      world.addComponent(ent1, Position{.x = 1.0F, .y = 1.0F});

      ecs::Entity ent2 = world.createEntity();
      world.addComponent(ent2, Velocity{.dx = 2.0F, .dy = 2.0F});

      // Query for Position + Velocity + Health (no entity has all three)
      ecs::ComponentSignature signature;
      signature.set(ecs::getComponentId<Position>());
      signature.set(ecs::getComponentId<Velocity>());
      signature.set(ecs::getComponentId<Health>());

      std::vector<ecs::Entity> entities;
      world.getEntitiesWithSignature(signature, entities);

      // No entities match
      CHECK(entities.empty());
    }

    SUBCASE("Edge case: no entities in world returns empty vector")
    {
      ecs::ComponentSignature signature;
      signature.set(ecs::getComponentId<Position>());

      std::vector<ecs::Entity> entities;
      world.getEntitiesWithSignature(signature, entities);

      // No entities exist
      CHECK(entities.empty());
    }

    SUBCASE("Multiple queries with the same vector reuse")
    {
      ecs::Entity ent1 = world.createEntity();
      world.addComponent(ent1, Position{.x = 1.0F, .y = 1.0F});

      ecs::Entity ent2 = world.createEntity();
      world.addComponent(ent2, Position{.x = 2.0F, .y = 2.0F});
      world.addComponent(ent2, Velocity{.dx = 3.0F, .dy = 3.0F});

      std::vector<ecs::Entity> entities;

      // First query: Position only
      ecs::ComponentSignature posSignature;
      posSignature.set(ecs::getComponentId<Position>());
      world.getEntitiesWithSignature(posSignature, entities);
      CHECK(entities.size() == 2);

      // Second query: Position + Velocity (should clear previous results)
      ecs::ComponentSignature posVelSignature;
      posVelSignature.set(ecs::getComponentId<Position>());
      posVelSignature.set(ecs::getComponentId<Velocity>());
      world.getEntitiesWithSignature(posVelSignature, entities);
      CHECK(entities.size() == 1);
      CHECK(entities[0] == ent2);
    }
  }
}
