/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** EntityManager Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/EntityManager.hpp"
#include <doctest/doctest.h>

// ============================================================================
// ENTITY MANAGER TESTS
// ============================================================================

TEST_SUITE("EntityManager")
{
  TEST_CASE("Create entities")
  {
    ecs::EntityManager manager;

    SUBCASE("Create single entity")
    {
      Entity entity = manager.createEntity();
      CHECK(entity == 0);
      CHECK(manager.isAlive(entity));
      CHECK(manager.getAliveCount() == 1);
    }

    SUBCASE("Create multiple entities")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      CHECK(ent0 == 0);
      CHECK(ent1 == 1);
      CHECK(ent2 == 2);

      CHECK(manager.isAlive(ent0));
      CHECK(manager.isAlive(ent1));
      CHECK(manager.isAlive(ent2));

      CHECK(manager.getAliveCount() == 3);
    }

    SUBCASE("Entity IDs are sequential")
    {
      constexpr int entityCount = 10;
      std::vector<Entity> entities;
      entities.reserve(entityCount);
      for (int i = 0; i < entityCount; ++i) {
        entities.push_back(manager.createEntity());
      }

      for (size_t i = 0; i < entities.size(); ++i) {
        CHECK(entities[i] == i);
      }
    }
  }

  TEST_CASE("Destroy entities")
  {
    ecs::EntityManager manager;

    SUBCASE("Destroy single entity")
    {
      Entity entity = manager.createEntity();
      CHECK(manager.isAlive(entity));

      manager.destroyEntity(entity);

      CHECK_FALSE(manager.isAlive(entity));
      CHECK(manager.getAliveCount() == 0);
    }

    SUBCASE("Destroy multiple entities")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      manager.destroyEntity(ent1);

      CHECK(manager.isAlive(ent0));
      CHECK_FALSE(manager.isAlive(ent1));
      CHECK(manager.isAlive(ent2));
      CHECK(manager.getAliveCount() == 2);
    }

    SUBCASE("Destroy non-existent entity (no crash)")
    {
      CHECK_NOTHROW(manager.destroyEntity(999));
    }

    SUBCASE("Destroy already destroyed entity (no crash)")
    {
      Entity entity = manager.createEntity();
      manager.destroyEntity(entity);
      CHECK_NOTHROW(manager.destroyEntity(entity));
    }
  }

  TEST_CASE("Entity ID recycling")
  {
    ecs::EntityManager manager;

    SUBCASE("Reuse destroyed entity ID")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      manager.destroyEntity(ent1);
      CHECK_FALSE(manager.isAlive(ent1));

      Entity ent3 = manager.createEntity();
      CHECK(ent3 == ent1); // Should reuse ent1's ID
      CHECK(manager.isAlive(ent3));
    }

    SUBCASE("LIFO order for recycled IDs")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      manager.destroyEntity(ent0);
      manager.destroyEntity(ent1);

      Entity ent3 = manager.createEntity(); // Should get ent1 (last destroyed)
      Entity ent4 = manager.createEntity(); // Should get ent0

      CHECK(ent3 == ent1);
      CHECK(ent4 == ent0);
    }
  }

  TEST_CASE("Component signatures")
  {
    ecs::EntityManager manager;

    SUBCASE("New entity has empty signature")
    {
      Entity entity = manager.createEntity();
      const auto &sig = manager.getSignature(entity);
      CHECK(sig.none());
    }

    SUBCASE("Set and get signature")
    {
      Entity entity = manager.createEntity();
      ecs::ComponentSignature signature;
      signature.set(0);
      signature.set(2);

      manager.setSignature(entity, signature);
      const auto &retrieved = manager.getSignature(entity);

      CHECK(retrieved.test(0));
      CHECK_FALSE(retrieved.test(1));
      CHECK(retrieved.test(2));
    }

    SUBCASE("Signature is reset when entity is destroyed")
    {
      Entity entity = manager.createEntity();
      ecs::ComponentSignature signature;
      signature.set(1);
      signature.set(3);

      manager.setSignature(entity, signature);
      manager.destroyEntity(entity);

      Entity newEntity = manager.createEntity(); // Reuses same ID
      const auto &sig = manager.getSignature(newEntity);
      CHECK(sig.none()); // Should be reset
    }

    SUBCASE("Get signature of non-existent entity throws")
    {
      CHECK_THROWS_AS((void)manager.getSignature(999), std::out_of_range);
    }

    SUBCASE("Set signature of non-existent entity throws")
    {
      ecs::ComponentSignature sig;
      CHECK_THROWS_AS(manager.setSignature(999, sig), std::out_of_range);
    }

    SUBCASE("Get signature of destroyed entity throws")
    {
      Entity entity = manager.createEntity();
      manager.destroyEntity(entity);
      CHECK_THROWS_AS((void)manager.getSignature(entity), std::out_of_range);
    }
  }

  TEST_CASE("Entity counting")
  {
    ecs::EntityManager manager;

    SUBCASE("Initial counts")
    {
      CHECK(manager.getAliveCount() == 0);
      CHECK(manager.getTotalCount() == 0);
    }

    SUBCASE("Count after creating entities")
    {
      (void)manager.createEntity();
      (void)manager.createEntity();
      (void)manager.createEntity();

      CHECK(manager.getAliveCount() == 3);
      CHECK(manager.getTotalCount() == 3);
    }

    SUBCASE("Count after destroying entities")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      manager.destroyEntity(ent1);

      CHECK(manager.getAliveCount() == 2);
      CHECK(manager.getTotalCount() == 3); // Total doesn't decrease
    }

    SUBCASE("Count after recycling")
    {
      Entity ent0 = manager.createEntity();
      manager.destroyEntity(ent0);
      (void)manager.createEntity(); // Reuses ent0

      CHECK(manager.getAliveCount() == 1);
      CHECK(manager.getTotalCount() == 1);
    }
  }

  TEST_CASE("Get all entities")
  {
    ecs::EntityManager manager;

    SUBCASE("Empty manager returns empty vector")
    {
      auto entities = manager.getAllEntities();
      CHECK(entities.empty());
    }

    SUBCASE("Returns all living entities")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      auto entities = manager.getAllEntities();
      CHECK(entities.size() == 3);
      CHECK(std::find(entities.begin(), entities.end(), ent0) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent1) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent2) != entities.end());
    }

    SUBCASE("Excludes destroyed entities")
    {
      Entity ent0 = manager.createEntity();
      Entity ent1 = manager.createEntity();
      Entity ent2 = manager.createEntity();

      manager.destroyEntity(ent1);

      auto entities = manager.getAllEntities();
      CHECK(entities.size() == 2);
      CHECK(std::find(entities.begin(), entities.end(), ent0) != entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent1) == entities.end());
      CHECK(std::find(entities.begin(), entities.end(), ent2) != entities.end());
    }
  }

  TEST_CASE("Clear manager")
  {
    ecs::EntityManager manager;

    SUBCASE("Clear resets all state")
    {
      (void)manager.createEntity();
      (void)manager.createEntity();
      (void)manager.createEntity();

      manager.clear();

      CHECK(manager.getAliveCount() == 0);
      CHECK(manager.getTotalCount() == 0);

      // New entities should start from 0 again
      Entity newEntity = manager.createEntity();
      CHECK(newEntity == 0);
    }
  }

  TEST_CASE("MAX_ENTITIES limit")
  {
    ecs::EntityManager manager;

    SUBCASE("Can create up to MAX_ENTITIES")
    {
      // This test would be slow with MAX_ENTITIES = 5000
      // Just verify the first few work
      constexpr int testEntityCount = 100;
      for (int i = 0; i < testEntityCount; ++i) {
        CHECK_NOTHROW((void)manager.createEntity());
      }
      CHECK(manager.getAliveCount() == testEntityCount);
    }

    SUBCASE("Creating more than MAX_ENTITIES throws (covers line 47)")
    {
      // Create ecs::MAX_ENTITIES entities
      for (size_t i = 0; i < ecs::MAX_ENTITIES; ++i) {
        (void)manager.createEntity();
      }

      // Next create should throw
      CHECK_THROWS_AS((void)manager.createEntity(), std::runtime_error);
    }
  }

  TEST_CASE("Entity state validation")
  {
    ecs::EntityManager manager;

    SUBCASE("isAlive returns false for non-existent entity")
    {
      CHECK_FALSE(manager.isAlive(999));
    }

    SUBCASE("isAlive returns false after destruction")
    {
      Entity entity = manager.createEntity();
      manager.destroyEntity(entity);
      CHECK_FALSE(manager.isAlive(entity));
    }

    SUBCASE("isAlive returns true for recycled entity")
    {
      Entity ent0 = manager.createEntity();
      manager.destroyEntity(ent0);
      Entity ent1 = manager.createEntity(); // Reuses ent0
      CHECK(manager.isAlive(ent1));
    }
  }
}
