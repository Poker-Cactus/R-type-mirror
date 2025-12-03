/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** SystemManager Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/ComponentSignature.hpp"
#include "ecs/ISystem.hpp"
#include "ecs/SystemManager.hpp"
#include "ecs/World.hpp"
#include <doctest/doctest.h>
#include <string>
#include <utility>

// ============================================================================
// TEST FIXTURES AND HELPER SYSTEMS
// ============================================================================

class TestSystem : public ecs::ISystem
{
private:
  int updateCallCount = 0;

public:
  void update(ecs::World &world, float deltaTime) override
  {
    updateCallCount++;
    (void)world;
    (void)deltaTime;
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

  [[nodiscard]] int getUpdateCallCount() const { return updateCallCount; }
};

class AnotherTestSystem : public ecs::ISystem
{
private:
  bool wasUpdated = false;

public:
  void update(ecs::World &world, float deltaTime) override
  {
    wasUpdated = true;
    (void)world;
    (void)deltaTime;
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

  [[nodiscard]] bool hasBeenUpdated() const { return wasUpdated; }
  void reset() { wasUpdated = false; }
};

class SystemWithConstructor : public ecs::ISystem
{
private:
  int value;
  std::string name;

public:
  SystemWithConstructor(int val, std::string n) : value(val), name(std::move(n)) {}

  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    (void)deltaTime;
  }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override { return {}; }

  [[nodiscard]] int getValue() const { return value; }
  [[nodiscard]] const std::string &getName() const { return name; }
};

// ============================================================================
// SYSTEM MANAGER TESTS
// ============================================================================

TEST_CASE("SystemManager - RegisterSystemCreatesNewSystem")
{
  ecs::SystemManager manager;
  ecs::World world;

  auto &system = manager.registerSystem<TestSystem>();

  CHECK(system.getUpdateCallCount() == 0);
}

TEST_CASE("SystemManager - RegisterMultipleSystemsSucceeds")
{
  ecs::SystemManager manager;
  ecs::World world;

  manager.registerSystem<TestSystem>();
  manager.registerSystem<AnotherTestSystem>();

  auto *testSys = manager.getSystem<TestSystem>();
  auto *anotherSys = manager.getSystem<AnotherTestSystem>();

  REQUIRE(testSys != nullptr);
  REQUIRE(anotherSys != nullptr);
}

TEST_CASE("SystemManager - RegisterSameSystemTwiceReturnsSameInstance")
{
  ecs::SystemManager manager;
  ecs::World world;

  auto &system1 = manager.registerSystem<TestSystem>();
  auto &system2 = manager.registerSystem<TestSystem>();

  CHECK(&system1 == &system2);
}

TEST_CASE("SystemManager - GetSystemReturnsValidPointer")
{
  ecs::SystemManager manager;
  ecs::World world;

  manager.registerSystem<TestSystem>();

  auto *system = manager.getSystem<TestSystem>();

  REQUIRE(system != nullptr);
}

TEST_CASE("SystemManager - GetNonExistentSystemReturnsNullptr")
{
  ecs::SystemManager manager;
  ecs::World world;

  auto *system = manager.getSystem<TestSystem>();

  CHECK(system == nullptr);
}

TEST_CASE("SystemManager - RemoveSystemDeletesIt")
{
  ecs::SystemManager manager;
  ecs::World world;

  manager.registerSystem<TestSystem>();

  auto *systemBefore = manager.getSystem<TestSystem>();
  REQUIRE(systemBefore != nullptr);

  manager.removeSystem<TestSystem>();

  auto *systemAfter = manager.getSystem<TestSystem>();
  CHECK(systemAfter == nullptr);
}

TEST_CASE("SystemManager - UpdateCallsAllRegisteredSystems")
{
  ecs::SystemManager manager;
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  manager.registerSystem<TestSystem>();
  manager.registerSystem<AnotherTestSystem>();

  manager.update(world, kDeltaTime);

  auto *testSys = manager.getSystem<TestSystem>();
  auto *anotherSys = manager.getSystem<AnotherTestSystem>();

  REQUIRE(testSys != nullptr);
  REQUIRE(anotherSys != nullptr);
  CHECK(testSys->getUpdateCallCount() == 1);
  CHECK(anotherSys->hasBeenUpdated());
}

TEST_CASE("SystemManager - UpdateMultipleTimesIncrementsCounter")
{
  ecs::SystemManager manager;
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  manager.registerSystem<TestSystem>();

  constexpr int kUpdateCount = 10;
  for (int i = 0; i < kUpdateCount; i++) {
    manager.update(world, kDeltaTime);
  }

  auto *system = manager.getSystem<TestSystem>();
  REQUIRE(system != nullptr);
  CHECK(system->getUpdateCallCount() == kUpdateCount);
}

TEST_CASE("SystemManager - SystemWithConstructorParamsInitializesCorrectly")
{
  ecs::SystemManager manager;
  ecs::World world;

  constexpr int kTestValue = 42;
  const std::string kTestName = "TestSystem";

  auto &system = manager.registerSystem<SystemWithConstructor>(kTestValue, kTestName);

  CHECK(system.getValue() == kTestValue);
  CHECK(system.getName() == kTestName);
}

TEST_CASE("SystemManager - RemovedSystemNotCalledOnUpdate")
{
  ecs::SystemManager manager;
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  manager.registerSystem<TestSystem>();
  manager.registerSystem<AnotherTestSystem>();

  manager.update(world, kDeltaTime);

  manager.removeSystem<TestSystem>();

  auto *anotherSys = manager.getSystem<AnotherTestSystem>();
  REQUIRE(anotherSys != nullptr);
  anotherSys->reset();

  manager.update(world, kDeltaTime);

  auto *testSys = manager.getSystem<TestSystem>();
  CHECK(testSys == nullptr);
  CHECK(anotherSys->hasBeenUpdated());
}

// ============================================================================
// WORLD INTEGRATION TESTS
// ============================================================================

TEST_CASE("WorldIntegration - RegisterSystemViaWorld")
{
  ecs::World world;

  world.registerSystem<TestSystem>();

  auto *system = world.getSystem<TestSystem>();
  REQUIRE(system != nullptr);
}

TEST_CASE("WorldIntegration - UpdateViaWorldCallsSystems")
{
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  world.registerSystem<TestSystem>();

  world.update(kDeltaTime);

  auto *system = world.getSystem<TestSystem>();
  REQUIRE(system != nullptr);
  CHECK(system->getUpdateCallCount() == 1);
}

TEST_CASE("WorldIntegration - RemoveSystemViaWorld")
{
  ecs::World world;

  world.registerSystem<TestSystem>();

  auto *systemBefore = world.getSystem<TestSystem>();
  REQUIRE(systemBefore != nullptr);

  world.removeSystem<TestSystem>();

  auto *systemAfter = world.getSystem<TestSystem>();
  CHECK(systemAfter == nullptr);
}

TEST_CASE("WorldIntegration - MultipleSystemsWorkTogether")
{
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  world.registerSystem<TestSystem>();
  world.registerSystem<AnotherTestSystem>();

  world.update(kDeltaTime);
  world.update(kDeltaTime);

  auto *testSys = world.getSystem<TestSystem>();
  auto *anotherSys = world.getSystem<AnotherTestSystem>();

  REQUIRE(testSys != nullptr);
  REQUIRE(anotherSys != nullptr);
  CHECK(testSys->getUpdateCallCount() == 2);
  CHECK(anotherSys->hasBeenUpdated());
}

TEST_CASE("WorldIntegration - RemovingOneSystemDoesNotAffectOthers")
{
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  world.registerSystem<TestSystem>();
  world.registerSystem<AnotherTestSystem>();

  world.update(kDeltaTime);
  world.removeSystem<TestSystem>();
  world.update(kDeltaTime);

  auto *testSys = world.getSystem<TestSystem>();
  auto *anotherSys = world.getSystem<AnotherTestSystem>();

  CHECK(testSys == nullptr);
  REQUIRE(anotherSys != nullptr);
  CHECK(anotherSys->hasBeenUpdated());
}

// ============================================================================
// EDGE CASES AND STRESS TESTS
// ============================================================================

TEST_CASE("EdgeCases - UpdateWithNoSystemsDoesNotCrash")
{
  ecs::SystemManager manager;
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  CHECK_NOTHROW(manager.update(world, kDeltaTime));
}

TEST_CASE("EdgeCases - RemoveNonExistentSystemDoesNotCrash")
{
  ecs::SystemManager manager;
  ecs::World world;

  CHECK_NOTHROW(manager.removeSystem<TestSystem>());
}

TEST_CASE("EdgeCases - GetSystemAfterMultipleRemoves")
{
  ecs::SystemManager manager;
  ecs::World world;

  manager.registerSystem<TestSystem>();

  manager.removeSystem<TestSystem>();
  manager.removeSystem<TestSystem>();
  manager.removeSystem<TestSystem>();

  auto *system = manager.getSystem<TestSystem>();
  CHECK(system == nullptr);
}

TEST_CASE("EdgeCases - ReRegisterAfterRemove")
{
  ecs::SystemManager manager;
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  auto &system1 = manager.registerSystem<TestSystem>();
  manager.update(world, kDeltaTime);

  CHECK(system1.getUpdateCallCount() == 1);

  manager.removeSystem<TestSystem>();

  auto &system2 = manager.registerSystem<TestSystem>();
  CHECK(system2.getUpdateCallCount() == 0); // New instance
}

TEST_CASE("EdgeCases - StressTestManyUpdates")
{
  ecs::SystemManager manager;
  ecs::World world;
  constexpr float kDeltaTime = 0.016F;

  manager.registerSystem<TestSystem>();

  constexpr int kManyUpdates = 1000;
  for (int i = 0; i < kManyUpdates; i++) {
    manager.update(world, kDeltaTime);
  }

  auto *system = manager.getSystem<TestSystem>();
  REQUIRE(system != nullptr);
  CHECK(system->getUpdateCallCount() == kManyUpdates);
}

// ============================================================================
// NEW FEATURES TESTS
// ============================================================================

TEST_CASE("NewFeatures - HasSystemReturnsTrueWhenRegistered")
{
  ecs::SystemManager manager;

  CHECK_FALSE(manager.hasSystem<TestSystem>());

  manager.registerSystem<TestSystem>();

  CHECK(manager.hasSystem<TestSystem>());
}

TEST_CASE("NewFeatures - HasSystemReturnsFalseAfterRemove")
{
  ecs::SystemManager manager;

  manager.registerSystem<TestSystem>();
  CHECK(manager.hasSystem<TestSystem>());

  manager.removeSystem<TestSystem>();
  CHECK_FALSE(manager.hasSystem<TestSystem>());
}

TEST_CASE("NewFeatures - GetSystemCountReturnsCorrectNumber")
{
  ecs::SystemManager manager;

  CHECK(manager.getSystemCount() == 0);

  manager.registerSystem<TestSystem>();
  CHECK(manager.getSystemCount() == 1);

  manager.registerSystem<AnotherTestSystem>();
  CHECK(manager.getSystemCount() == 2);

  manager.removeSystem<TestSystem>();
  CHECK(manager.getSystemCount() == 1);
}

TEST_CASE("NewFeatures - ClearRemovesAllSystems")
{
  ecs::SystemManager manager;
  constexpr int kTestValue = 42;

  manager.registerSystem<TestSystem>();
  manager.registerSystem<AnotherTestSystem>();
  manager.registerSystem<SystemWithConstructor>(kTestValue, "test");

  CHECK(manager.getSystemCount() == 3);

  manager.clear();

  CHECK(manager.getSystemCount() == 0);
  CHECK_FALSE(manager.hasSystem<TestSystem>());
  CHECK_FALSE(manager.hasSystem<AnotherTestSystem>());
  CHECK_FALSE(manager.hasSystem<SystemWithConstructor>());
}

TEST_CASE("NewFeatures - ConstGetSystemWorks")
{
  ecs::SystemManager manager;
  manager.registerSystem<TestSystem>();

  const auto &constManager = manager;
  const auto *system = constManager.getSystem<TestSystem>();

  REQUIRE(system != nullptr);
  CHECK(system->getUpdateCallCount() == 0);
}

TEST_CASE("NewFeatures - WorldHasSystemDelegates")
{
  ecs::World world;

  CHECK_FALSE(world.hasSystem<TestSystem>());

  world.registerSystem<TestSystem>();

  CHECK(world.hasSystem<TestSystem>());
}

TEST_CASE("NewFeatures - WorldGetSystemCountWorks")
{
  ecs::World world;

  CHECK(world.getSystemCount() == 0);

  world.registerSystem<TestSystem>();
  world.registerSystem<AnotherTestSystem>();

  CHECK(world.getSystemCount() == 2);
}

TEST_CASE("NewFeatures - WorldClearSystemsWorks")
{
  ecs::World world;

  world.registerSystem<TestSystem>();
  world.registerSystem<AnotherTestSystem>();

  CHECK(world.getSystemCount() == 2);

  world.clearSystems();

  CHECK(world.getSystemCount() == 0);
}

TEST_CASE("NewFeatures - WorldConstGetSystemWorks")
{
  ecs::World world;
  world.registerSystem<TestSystem>();

  const auto &constWorld = world;
  const auto *system = constWorld.getSystem<TestSystem>();

  REQUIRE(system != nullptr);
  CHECK(system->getUpdateCallCount() == 0);
}
