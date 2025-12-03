/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ComponentSignature Unit Tests with doctest
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ecs/ComponentManager.hpp"
#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include "ecs/ISystem.hpp"
#include "ecs/World.hpp"
#include <doctest/doctest.h>

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct Position {
  float x;
  float y;
};

struct Velocity {
  float vx;
  float vy;
};

struct Health {
  int current;
  int max;
};

struct Sprite {
  int textureId;
};

// ============================================================================
// TEST SYSTEMS WITH SIGNATURES
// ============================================================================

class PositionVelocitySystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    (void)deltaTime;
  }

  ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<Position>());
    sig.set(ecs::getComponentId<Velocity>());
    return sig;
  }
};

class HealthSystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    (void)deltaTime;
  }

  ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<Health>());
    return sig;
  }
};

class RenderSystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    (void)world;
    (void)deltaTime;
  }

  ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<Position>());
    sig.set(ecs::getComponentId<Sprite>());
    return sig;
  }
};

// ============================================================================
// COMPONENT ID TESTS
// ============================================================================

TEST_CASE("ComponentId - GetComponentIdIsStable")
{
  auto id1 = ecs::getComponentId<Position>();
  auto id2 = ecs::getComponentId<Position>();
  auto id3 = ecs::getComponentId<Velocity>();

  CHECK(id1 == id2);
  CHECK(id1 != id3);
}

TEST_CASE("ComponentId - DifferentTypesHaveDifferentIds")
{
  auto posId = ecs::getComponentId<Position>();
  auto velId = ecs::getComponentId<Velocity>();
  auto hpId = ecs::getComponentId<Health>();
  auto spriteId = ecs::getComponentId<Sprite>();

  CHECK(posId != velId);
  CHECK(posId != hpId);
  CHECK(posId != spriteId);
  CHECK(velId != hpId);
  CHECK(velId != spriteId);
  CHECK(hpId != spriteId);
}

TEST_CASE("ComponentId - IdsFitWithinBitsetSize")
{
  auto posId = ecs::getComponentId<Position>();
  auto velId = ecs::getComponentId<Velocity>();
  auto hpId = ecs::getComponentId<Health>();
  auto spriteId = ecs::getComponentId<Sprite>();

  CHECK(posId < ecs::MAX_COMPONENTS);
  CHECK(velId < ecs::MAX_COMPONENTS);
  CHECK(hpId < ecs::MAX_COMPONENTS);
  CHECK(spriteId < ecs::MAX_COMPONENTS);
}

// ============================================================================
// SIGNATURE BITSET TESTS
// ============================================================================

TEST_CASE("Signature - EmptySignatureHasNoComponents")
{
  ecs::ComponentSignature sig;

  CHECK(sig.none());
  CHECK(sig.count() == 0);
}

TEST_CASE("Signature - SetBitActivatesComponent")
{
  ecs::ComponentSignature sig;

  sig.set(ecs::getComponentId<Position>());

  CHECK(sig.test(ecs::getComponentId<Position>()));
  CHECK(sig.count() == 1);
}

TEST_CASE("Signature - SetMultipleBitsWorks")
{
  ecs::ComponentSignature sig;

  sig.set(ecs::getComponentId<Position>());
  sig.set(ecs::getComponentId<Velocity>());
  sig.set(ecs::getComponentId<Health>());

  CHECK(sig.test(ecs::getComponentId<Position>()));
  CHECK(sig.test(ecs::getComponentId<Velocity>()));
  CHECK(sig.test(ecs::getComponentId<Health>()));
  CHECK(sig.count() == 3);
}

TEST_CASE("Signature - ResetBitDeactivatesComponent")
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<Position>());
  sig.set(ecs::getComponentId<Velocity>());

  sig.reset(ecs::getComponentId<Position>());

  CHECK_FALSE(sig.test(ecs::getComponentId<Position>()));
  CHECK(sig.test(ecs::getComponentId<Velocity>()));
  CHECK(sig.count() == 1);
}

TEST_CASE("Signature - ResetAllClearsSignature")
{
  ecs::ComponentSignature sig;
  sig.set(ecs::getComponentId<Position>());
  sig.set(ecs::getComponentId<Velocity>());

  sig.reset();

  CHECK(sig.none());
  CHECK(sig.count() == 0);
}

// ============================================================================
// BITWISE OPERATION TESTS
// ============================================================================

TEST_CASE("BitwiseOps - AndOperationMatchesComponents")
{
  ecs::ComponentSignature entitySig;
  entitySig.set(ecs::getComponentId<Position>());
  entitySig.set(ecs::getComponentId<Velocity>());
  entitySig.set(ecs::getComponentId<Health>());

  ecs::ComponentSignature systemSig;
  systemSig.set(ecs::getComponentId<Position>());
  systemSig.set(ecs::getComponentId<Velocity>());

  // Entity has all required components
  CHECK((entitySig & systemSig) == systemSig);
}

TEST_CASE("BitwiseOps - AndOperationFailsWhenComponentMissing")
{
  ecs::ComponentSignature entitySig;
  entitySig.set(ecs::getComponentId<Position>());
  entitySig.set(ecs::getComponentId<Health>());

  ecs::ComponentSignature systemSig;
  systemSig.set(ecs::getComponentId<Position>());
  systemSig.set(ecs::getComponentId<Velocity>());

  // Entity is missing Velocity
  CHECK((entitySig & systemSig) != systemSig);
}

TEST_CASE("BitwiseOps - OrOperationCombinesSignatures")
{
  ecs::ComponentSignature sig1;
  sig1.set(ecs::getComponentId<Position>());

  ecs::ComponentSignature sig2;
  sig2.set(ecs::getComponentId<Velocity>());

  auto combined = sig1 | sig2;

  CHECK(combined.test(ecs::getComponentId<Position>()));
  CHECK(combined.test(ecs::getComponentId<Velocity>()));
}

TEST_CASE("BitwiseOps - EqualityComparison")
{
  ecs::ComponentSignature sig1;
  sig1.set(ecs::getComponentId<Position>());
  sig1.set(ecs::getComponentId<Velocity>());

  ecs::ComponentSignature sig2;
  sig2.set(ecs::getComponentId<Position>());
  sig2.set(ecs::getComponentId<Velocity>());

  ecs::ComponentSignature sig3;
  sig3.set(ecs::getComponentId<Position>());

  CHECK(sig1 == sig2);
  CHECK(sig1 != sig3);
}

// ============================================================================
// SYSTEM SIGNATURE TESTS
// ============================================================================

TEST_CASE("SystemSignature - PhysicsSystemRequiresPositionAndVelocity")
{
  PositionVelocitySystem system;
  auto signature = system.getSignature();

  CHECK(signature.test(ecs::getComponentId<Position>()));
  CHECK(signature.test(ecs::getComponentId<Velocity>()));
  CHECK_FALSE(signature.test(ecs::getComponentId<Health>()));
  CHECK_FALSE(signature.test(ecs::getComponentId<Sprite>()));
}

TEST_CASE("SystemSignature - HealthSystemRequiresOnlyHealth")
{
  HealthSystem system;
  auto signature = system.getSignature();

  CHECK(signature.test(ecs::getComponentId<Health>()));
  CHECK_FALSE(signature.test(ecs::getComponentId<Position>()));
  CHECK_FALSE(signature.test(ecs::getComponentId<Velocity>()));
}

TEST_CASE("SystemSignature - RenderSystemRequiresPositionAndSprite")
{
  RenderSystem system;
  auto signature = system.getSignature();

  CHECK(signature.test(ecs::getComponentId<Position>()));
  CHECK(signature.test(ecs::getComponentId<Sprite>()));
  CHECK_FALSE(signature.test(ecs::getComponentId<Velocity>()));
  CHECK_FALSE(signature.test(ecs::getComponentId<Health>()));
}

TEST_CASE("SystemSignature - DifferentSystemsHaveDifferentSignatures")
{
  PositionVelocitySystem physicsSystem;
  HealthSystem healthSystem;
  RenderSystem renderSystem;

  auto physicsSig = physicsSystem.getSignature();
  auto healthSig = healthSystem.getSignature();
  auto renderSig = renderSystem.getSignature();

  CHECK(physicsSig != healthSig);
  CHECK(physicsSig != renderSig);
  CHECK(healthSig != renderSig);
}

// ============================================================================
// COMPONENT MANAGER SIGNATURE TESTS
// ============================================================================

TEST_CASE("ComponentManager - TracksEntitySignature")
{
  ComponentManager manager;
  constexpr Entity kTestEntity = 42;

  auto sigBefore = manager.getEntitySignature(kTestEntity);
  CHECK(sigBefore.none());

  constexpr float kTestPosY = 20.0F;
  constexpr float kTestPosX = 10.0F;
  manager.addComponent(kTestEntity, Position{.x = kTestPosX, .y = kTestPosY});

  auto sigAfter = manager.getEntitySignature(kTestEntity);
  CHECK(sigAfter.test(ecs::getComponentId<Position>()));
  CHECK_FALSE(sigAfter.test(ecs::getComponentId<Velocity>()));
}

TEST_CASE("ComponentManager - UpdatesSignatureOnComponentAdd")
{
  ComponentManager manager;
  constexpr Entity kEntity = 1;

  manager.addComponent(kEntity, Position{0, 0});
  manager.addComponent(kEntity, Velocity{1, 1});

  auto sig = manager.getEntitySignature(kEntity);

  CHECK(sig.test(ecs::getComponentId<Position>()));
  CHECK(sig.test(ecs::getComponentId<Velocity>()));
  CHECK(sig.count() == 2);
}

TEST_CASE("ComponentManager - UpdatesSignatureOnComponentRemove")
{
  ComponentManager manager;
  constexpr Entity kEntity = 1;

  manager.addComponent(kEntity, Position{0, 0});
  manager.addComponent(kEntity, Velocity{1, 1});
  manager.removeComponent<Velocity>(kEntity);

  auto sig = manager.getEntitySignature(kEntity);

  CHECK(sig.test(ecs::getComponentId<Position>()));
  CHECK_FALSE(sig.test(ecs::getComponentId<Velocity>()));
  CHECK(sig.count() == 1);
}

TEST_CASE("ComponentManager - ClearsSignatureOnRemoveAllComponents")
{
  ComponentManager manager;
  constexpr Entity kEntity = 1;
  constexpr int kMaxHealth = 100;

  manager.addComponent(kEntity, Position{0, 0});
  manager.addComponent(kEntity, Velocity{1, 1});
  manager.addComponent(kEntity, Health{kMaxHealth, kMaxHealth});

  manager.removeAllComponents(kEntity);

  auto sig = manager.getEntitySignature(kEntity);
  CHECK(sig.none());
}

TEST_CASE("ComponentManager - MultipleEntitiesHaveIndependentSignatures")
{
  ComponentManager manager;
  constexpr Entity kEntity1 = 1;
  constexpr Entity kEntity2 = 2;

  manager.addComponent(kEntity1, Position{0, 0});
  manager.addComponent(kEntity2, Velocity{1, 1});

  auto sig1 = manager.getEntitySignature(kEntity1);
  auto sig2 = manager.getEntitySignature(kEntity2);

  CHECK(sig1.test(ecs::getComponentId<Position>()));
  CHECK_FALSE(sig1.test(ecs::getComponentId<Velocity>()));

  CHECK_FALSE(sig2.test(ecs::getComponentId<Position>()));
  CHECK(sig2.test(ecs::getComponentId<Velocity>()));
}

// ============================================================================
// WORLD INTEGRATION TESTS
// ============================================================================

TEST_CASE("WorldIntegration - ProvidesEntitySignatureAccess")
{
  ecs::World world;
  constexpr Entity kEntity = 5;
  constexpr float kPosX = 5.0F;
  constexpr float kPosY = 10.0F;

  world.getComponentManager().addComponent(kEntity, Position{kPosX, kPosY});

  auto sig = world.getEntitySignature(kEntity);

  CHECK(sig.test(ecs::getComponentId<Position>()));
}

TEST_CASE("WorldIntegration - SignaturePersistsAcrossComponentOperations")
{
  ecs::World world;
  constexpr Entity kEntity = 1;
  constexpr int kMaxHealth = 100;

  auto &manager = world.getComponentManager();

  manager.addComponent(kEntity, Position{0, 0});
  CHECK(world.getEntitySignature(kEntity).count() == 1);

  manager.addComponent(kEntity, Velocity{0, 0});
  CHECK(world.getEntitySignature(kEntity).count() == 2);

  manager.addComponent(kEntity, Health{kMaxHealth, kMaxHealth});
  CHECK(world.getEntitySignature(kEntity).count() == 3);

  manager.removeComponent<Velocity>(kEntity);
  CHECK(world.getEntitySignature(kEntity).count() == 2);
}

// ============================================================================
// ENTITY FILTERING TESTS
// ============================================================================

TEST_CASE("EntityFiltering - MatchesSystemRequirements")
{
  ecs::ComponentSignature entitySig;
  entitySig.set(ecs::getComponentId<Position>());
  entitySig.set(ecs::getComponentId<Velocity>());
  entitySig.set(ecs::getComponentId<Sprite>());

  PositionVelocitySystem physicsSystem;
  RenderSystem renderSystem;
  HealthSystem healthSystem;

  auto physicsSig = physicsSystem.getSignature();
  auto renderSig = renderSystem.getSignature();
  auto healthSig = healthSystem.getSignature();

  // Entity matches physics (has Position + Velocity)
  CHECK((entitySig & physicsSig) == physicsSig);

  // Entity matches render (has Position + Sprite)
  CHECK((entitySig & renderSig) == renderSig);

  // Entity doesn't match health (missing Health)
  CHECK((entitySig & healthSig) != healthSig);
}

TEST_CASE("EntityFiltering - PartialMatchIsNotEnough")
{
  ecs::ComponentSignature entitySig;
  entitySig.set(ecs::getComponentId<Position>());
  // Missing Velocity

  PositionVelocitySystem system;
  auto systemSig = system.getSignature();

  // Entity has Position but not Velocity - should NOT match
  CHECK((entitySig & systemSig) != systemSig);
}

TEST_CASE("EntityFiltering - ExtraComponentsDoNotPreventMatch")
{
  ecs::ComponentSignature entitySig;
  entitySig.set(ecs::getComponentId<Position>());
  entitySig.set(ecs::getComponentId<Velocity>());
  entitySig.set(ecs::getComponentId<Health>());
  entitySig.set(ecs::getComponentId<Sprite>());

  PositionVelocitySystem system;
  auto systemSig = system.getSignature();

  // Entity has Position + Velocity + extras - should still match
  CHECK((entitySig & systemSig) == systemSig);
}
