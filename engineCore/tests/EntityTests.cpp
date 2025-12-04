/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Entity Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/Entity.hpp"
#include <cstdint>
#include <doctest/doctest.h>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <vector>

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
      constexpr Entity testEntity1 = 42;
      constexpr Entity testEntity2 = 100;
      Entity entity1 = testEntity1;
      Entity entity2 = testEntity1;
      Entity entity3 = testEntity2;

      CHECK(entity1 == entity2);
      CHECK(entity1 != entity3);
      CHECK(entity1 < entity3);
    }

    SUBCASE("Arithmetic operations")
    {
      constexpr Entity initialValue = 10;
      constexpr Entity addValue = 5;
      Entity entity = initialValue;
      entity++;
      CHECK(entity == 11);

      entity += addValue;
      CHECK(entity == 16);

      entity--;
      CHECK(entity == 15);
    }

    SUBCASE("Maximum entity value")
    {
      Entity maxEntity = std::numeric_limits<std::uint32_t>::max();
      CHECK(maxEntity == 4294967295U);
    }

    SUBCASE("Entity ID range")
    {
      constexpr Entity midValue = 1000;
      Entity minEntity = 0;
      Entity midEntity = midValue;
      Entity maxEntity = std::numeric_limits<std::uint32_t>::max();

      CHECK(minEntity < midEntity);
      CHECK(midEntity < maxEntity);
    }
  }

  TEST_CASE("Entity as container key")
  {
    SUBCASE("Can be used in std::unordered_map")
    {
      constexpr int value1 = 100;
      constexpr int value2 = 200;
      constexpr Entity key3 = 42;
      constexpr int value3 = 300;
      std::unordered_map<Entity, int> entityMap;
      entityMap[0] = value1;
      entityMap[1] = value2;
      entityMap[key3] = value3;

      CHECK(entityMap[0] == value1);
      CHECK(entityMap[1] == value2);
      CHECK(entityMap[key3] == value3);
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
