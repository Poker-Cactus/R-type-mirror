/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Entity Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/Entity.hpp"
#include <doctest/doctest.h>
#include <limits>

// ============================================================================
// ENTITY TYPE TESTS
// ============================================================================

TEST_SUITE("Entity")
{
  TEST_CASE("Entity is uint32_t")
  {
    SUBCASE("Type checking")
    {
      CHECK(std::is_same<Entity, std::uint32_t>::value);
    }

    SUBCASE("Default value initialization")
    {
      Entity entity = 0;
      CHECK(entity == 0);
    }

    SUBCASE("Assignment and comparison")
    {
      Entity entity1 = 42;
      Entity entity2 = 42;
      Entity entity3 = 100;

      CHECK(entity1 == entity2);
      CHECK(entity1 != entity3);
      CHECK(entity1 < entity3);
    }

    SUBCASE("Arithmetic operations")
    {
      Entity entity = 10;
      entity++;
      CHECK(entity == 11);

      entity += 5;
      CHECK(entity == 16);

      entity--;
      CHECK(entity == 15);
    }

    SUBCASE("Maximum entity value")
    {
      Entity maxEntity = std::numeric_limits<std::uint32_t>::max();
      CHECK(maxEntity == 4294967295u);
    }

    SUBCASE("Entity ID range")
    {
      Entity minEntity = 0;
      Entity midEntity = 1000;
      Entity maxEntity = std::numeric_limits<std::uint32_t>::max();

      CHECK(minEntity < midEntity);
      CHECK(midEntity < maxEntity);
    }
  }

  TEST_CASE("Entity as container key")
  {
    SUBCASE("Can be used in std::unordered_map")
    {
      std::unordered_map<Entity, int> entityMap;
      entityMap[0] = 100;
      entityMap[1] = 200;
      entityMap[42] = 300;

      CHECK(entityMap[0] == 100);
      CHECK(entityMap[1] == 200);
      CHECK(entityMap[42] == 300);
      CHECK(entityMap.size() == 3);
    }

    SUBCASE("Can be used in std::vector")
    {
      std::vector<Entity> entities = {0, 1, 2, 3, 4};
      CHECK(entities.size() == 5);
      CHECK(entities[0] == 0);
      CHECK(entities[4] == 4);
    }
  }
}
